# -*- perl -*-
# Text::Template.pm
#
# Fill in `templates'
#
# Copyright 1996, 1997, 1999, 2001, 2002, 2003, 2008 M-J. Dominus.
# You may copy and distribute this program under the
# same terms as Perl iteself.
# If in doubt, write to mjd-perl-template+@plover.com for a license.
#
# Version 1.45

# DOCS HAVE BEEN REMOVED FROM THIS FILE

package Text::Template;
require 5.004;
use Exporter;
@ISA = qw(Exporter);
@EXPORT_OK = qw(fill_in_file fill_in_string TTerror);
use vars '$ERROR';
use strict;

$Text::Template::VERSION = '1.45';
my %GLOBAL_PREPEND = ('Text::Template' => '');

sub Version {
  $Text::Template::VERSION;
}

sub _param {
  my $kk;
  my ($k, %h) = @_;
  for $kk ($k, "\u$k", "\U$k", "-$k", "-\u$k", "-\U$k") {
    return $h{$kk} if exists $h{$kk};
  }
  return;
}

sub always_prepend
{
  my $pack = shift;
  my $old = $GLOBAL_PREPEND{$pack};
  $GLOBAL_PREPEND{$pack} = shift;
  $old;
}

{
  my %LEGAL_TYPE;
  BEGIN {
    %LEGAL_TYPE = map {$_=>1} qw(FILE FILEHANDLE STRING ARRAY);
  }
  sub new {
    my $pack = shift;
    my %a = @_;
    my $stype = uc(_param('type', %a)) || 'FILE';
    my $source = _param('source', %a);
    my $untaint = _param('untaint', %a);
    my $prepend = _param('prepend', %a);
    my $alt_delim = _param('delimiters', %a);
    my $broken = _param('broken', %a);
    unless (defined $source) {
      require Carp;
      Carp::croak("Usage: $ {pack}::new(TYPE => ..., SOURCE => ...)");
    }
    unless ($LEGAL_TYPE{$stype}) {
      require Carp;
      Carp::croak("Illegal value `$stype' for TYPE parameter");
    }
    my $self = {TYPE => $stype,
		PREPEND => $prepend,
                UNTAINT => $untaint,
                BROKEN => $broken,
		(defined $alt_delim ? (DELIM => $alt_delim) : ()),
	       };
    # Under 5.005_03, if any of $stype, $prepend, $untaint, or $broken
    # are tainted, all the others become tainted too as a result of
    # sharing the expression with them.  We install $source separately
    # to prevent it from acquiring a spurious taint.
    $self->{SOURCE} = $source;

    bless $self => $pack;
    return unless $self->_acquire_data;

    $self;
  }
}

# Convert template objects of various types to type STRING,
# in which the template data is embedded in the object itself.
sub _acquire_data {
  my ($self) = @_;
  my $type = $self->{TYPE};
  if ($type eq 'STRING') {
    # nothing necessary
  } elsif ($type eq 'FILE') {
    my $data = _load_text($self->{SOURCE});
    unless (defined $data) {
      # _load_text already set $ERROR
      return undef;
    }
    if ($self->{UNTAINT} && _is_clean($self->{SOURCE})) {
      _unconditionally_untaint($data);
    }
    $self->{TYPE} = 'STRING';
    $self->{FILENAME} = $self->{SOURCE};
    $self->{SOURCE} = $data;
  } elsif ($type eq 'ARRAY') {
    $self->{TYPE} = 'STRING';
    $self->{SOURCE} = join '', @{$self->{SOURCE}};
  } elsif ($type eq 'FILEHANDLE') {
    $self->{TYPE} = 'STRING';
    local $/;
    my $fh = $self->{SOURCE};
    my $data = <$fh>; # Extra assignment avoids bug in Solaris perl5.00[45].
    if ($self->{UNTAINT}) {
      _unconditionally_untaint($data);
    }
    $self->{SOURCE} = $data;
  } else {
    # This should have been caught long ago, so it represents a
    # drastic `can't-happen' sort of failure
    my $pack = ref $self;
    die "Can only acquire data for $pack objects of subtype STRING, but this is $type; aborting";
  }
  $self->{DATA_ACQUIRED} = 1;
}

sub source {
  my ($self) = @_;
  $self->_acquire_data unless $self->{DATA_ACQUIRED};
  return $self->{SOURCE};
}

sub set_source_data {
  my ($self, $newdata) = @_;
  $self->{SOURCE} = $newdata;
  $self->{DATA_ACQUIRED} = 1;
  $self->{TYPE} = 'STRING';
  1;
}

sub compile {
  my $self = shift;

  return 1 if $self->{TYPE} eq 'PREPARSED';

  return undef unless $self->_acquire_data;
  unless ($self->{TYPE} eq 'STRING') {
    my $pack = ref $self;
    # This should have been caught long ago, so it represents a
    # drastic `can't-happen' sort of failure
    die "Can only compile $pack objects of subtype STRING, but this is $self->{TYPE}; aborting";
  }

  my @tokens;
  my $delim_pats = shift() || $self->{DELIM};



  my ($t_open, $t_close) = ('{', '}');
  my $DELIM;			# Regex matches a delimiter if $delim_pats
  if (defined $delim_pats) {
    ($t_open, $t_close) = @$delim_pats;
    $DELIM = "(?:(?:\Q$t_open\E)|(?:\Q$t_close\E))";
    @tokens = split /($DELIM|\n)/, $self->{SOURCE};
  } else {
    @tokens = split /(\\\\(?=\\*[{}])|\\[{}]|[{}\n])/, $self->{SOURCE};
  }
  my $state = 'TEXT';
  my $depth = 0;
  my $lineno = 1;
  my @content;
  my $cur_item = '';
  my $prog_start;
  while (@tokens) {
    my $t = shift @tokens;
    next if $t eq '';
    if ($t eq $t_open) {	# Brace or other opening delimiter
      if ($depth == 0) {
	push @content, [$state, $cur_item, $lineno] if $cur_item ne '';
	$cur_item = '';
	$state = 'PROG';
	$prog_start = $lineno;
      } else {
	$cur_item .= $t;
      }
      $depth++;
    } elsif ($t eq $t_close) {	# Brace or other closing delimiter
      $depth--;
      if ($depth < 0) {
	$ERROR = "Unmatched close brace at line $lineno";
	return undef;
      } elsif ($depth == 0) {
	push @content, [$state, $cur_item, $prog_start] if $cur_item ne '';
	$state = 'TEXT';
	$cur_item = '';
      } else {
	$cur_item .= $t;
      }
    } elsif (!$delim_pats && $t eq '\\\\') { # precedes \\\..\\\{ or \\\..\\\}
      $cur_item .= '\\';
    } elsif (!$delim_pats && $t =~ /^\\([{}])$/) { # Escaped (literal) brace?
	$cur_item .= $1;
    } elsif ($t eq "\n") {	# Newline
      $lineno++;
      $cur_item .= $t;
    } else {			# Anything else
      $cur_item .= $t;
    }
  }

  if ($state eq 'PROG') {
    $ERROR = "End of data inside program text that began at line $prog_start";
    return undef;
  } elsif ($state eq 'TEXT') {
    push @content, [$state, $cur_item, $lineno] if $cur_item ne '';
  } else {
    die "Can't happen error #1";
  }

  $self->{TYPE} = 'PREPARSED';
  $self->{SOURCE} = \@content;
  1;
}

sub prepend_text {
  my ($self) = @_;
  my $t = $self->{PREPEND};
  unless (defined $t) {
    $t = $GLOBAL_PREPEND{ref $self};
    unless (defined $t) {
      $t = $GLOBAL_PREPEND{'Text::Template'};
    }
  }
  $self->{PREPEND} = $_[1] if $#_ >= 1;
  return $t;
}

sub fill_in {
  my $fi_self = shift;
  my %fi_a = @_;

  unless ($fi_self->{TYPE} eq 'PREPARSED') {
    my $delims = _param('delimiters', %fi_a);
    my @delim_arg = (defined $delims ? ($delims) : ());
    $fi_self->compile(@delim_arg)
      or return undef;
  }

  my $fi_varhash = _param('hash', %fi_a);
  my $fi_package = _param('package', %fi_a) ;
  my $fi_broken  =
    _param('broken', %fi_a)  || $fi_self->{BROKEN} || \&_default_broken;
  my $fi_broken_arg = _param('broken_arg', %fi_a) || [];
  my $fi_safe = _param('safe', %fi_a);
  my $fi_ofh = _param('output', %fi_a);
  my $fi_eval_package;
  my $fi_scrub_package = 0;
  my $fi_filename = _param('filename') || $fi_self->{FILENAME} || 'template';

  my $fi_prepend = _param('prepend', %fi_a);
  unless (defined $fi_prepend) {
    $fi_prepend = $fi_self->prepend_text;
  }

  if (defined $fi_safe) {
    $fi_eval_package = 'main';
  } elsif (defined $fi_package) {
    $fi_eval_package = $fi_package;
  } elsif (defined $fi_varhash) {
    $fi_eval_package = _gensym();
    $fi_scrub_package = 1;
  } else {
    $fi_eval_package = caller;
  }

  my $fi_install_package;
  if (defined $fi_varhash) {
    if (defined $fi_package) {
      $fi_install_package = $fi_package;
    } elsif (defined $fi_safe) {
      $fi_install_package = $fi_safe->root;
    } else {
      $fi_install_package = $fi_eval_package; # The gensymmed one
    }
    _install_hash($fi_varhash => $fi_install_package);
  }

  if (defined $fi_package && defined $fi_safe) {
    no strict 'refs';
    # Big fat magic here: Fix it so that the user-specified package
    # is the default one available in the safe compartment.
    *{$fi_safe->root . '::'} = \%{$fi_package . '::'};   # LOD
  }

  my $fi_r = '';
  my $fi_item;
  foreach $fi_item (@{$fi_self->{SOURCE}}) {
    my ($fi_type, $fi_text, $fi_lineno) = @$fi_item;
    if ($fi_type eq 'TEXT') {
      if ($fi_ofh) {
	print $fi_ofh $fi_text;
      } else {
	$fi_r .= $fi_text;
      }
    } elsif ($fi_type eq 'PROG') {
      no strict;
      my $fi_lcomment = "#line $fi_lineno $fi_filename";
      my $fi_progtext =
        "package $fi_eval_package; $fi_prepend;\n$fi_lcomment\n$fi_text;";
      my $fi_res;
      my $fi_eval_err = '';
      if ($fi_safe) {
        $fi_safe->reval(q{undef $OUT});
	$fi_res = $fi_safe->reval($fi_progtext);
	$fi_eval_err = $@;
	my $OUT = $fi_safe->reval('$OUT');
	$fi_res = $OUT if defined $OUT;
      } else {
	my $OUT;
	$fi_res = eval $fi_progtext;
	$fi_eval_err = $@;
	$fi_res = $OUT if defined $OUT;
      }

      # If the value of the filled-in text really was undef,
      # change it to an explicit empty string to avoid undefined
      # value warnings later.
      $fi_res = '' unless defined $fi_res;

      if ($fi_eval_err) {
	$fi_res = $fi_broken->(text => $fi_text,
			       error => $fi_eval_err,
			       lineno => $fi_lineno,
			       arg => $fi_broken_arg,
			       );
	if (defined $fi_res) {
	  if (defined $fi_ofh) {
	    print $fi_ofh $fi_res;
	  } else {
	    $fi_r .= $fi_res;
	  }
	} else {
	  return $fi_res;		# Undefined means abort processing
	}
      } else {
	if (defined $fi_ofh) {
	  print $fi_ofh $fi_res;
	} else {
	  $fi_r .= $fi_res;
	}
      }
    } else {
      die "Can't happen error #2";
    }
  }

  _scrubpkg($fi_eval_package) if $fi_scrub_package;
  defined $fi_ofh ? 1 : $fi_r;
}

sub fill_this_in {
  my $pack = shift;
  my $text = shift;
  my $templ = $pack->new(TYPE => 'STRING', SOURCE => $text, @_)
    or return undef;
  $templ->compile or return undef;
  my $result = $templ->fill_in(@_);
  $result;
}

sub fill_in_string {
  my $string = shift;
  my $package = _param('package', @_);
  push @_, 'package' => scalar(caller) unless defined $package;
  Text::Template->fill_this_in($string, @_);
}

sub fill_in_file {
  my $fn = shift;
  my $templ = Text::Template->new(TYPE => 'FILE', SOURCE => $fn, @_)
    or return undef;
  $templ->compile or return undef;
  my $text = $templ->fill_in(@_);
  $text;
}

sub _default_broken {
  my %a = @_;
  my $prog_text = $a{text};
  my $err = $a{error};
  my $lineno = $a{lineno};
  chomp $err;
#  $err =~ s/\s+at .*//s;
  "Program fragment delivered error ``$err''";
}

sub _load_text {
  my $fn = shift;
  local *F;
  unless (open F, $fn) {
    $ERROR = "Couldn't open file $fn: $!";
    return undef;
  }
  local $/;
  <F>;
}

sub _is_clean {
  my $z;
  eval { ($z = join('', @_)), eval '#' . substr($z,0,0); 1 }   # LOD
}

sub _unconditionally_untaint {
  for (@_) {
    ($_) = /(.*)/s;
  }
}

{
  my $seqno = 0;
  sub _gensym {
    __PACKAGE__ . '::GEN' . $seqno++;
  }
  sub _scrubpkg {
    my $s = shift;
    $s =~ s/^Text::Template:://;
    no strict 'refs';
    my $hash = $Text::Template::{$s."::"};
    foreach my $key (keys %$hash) {
      undef $hash->{$key};
    }
  }
}

# Given a hashful of variables (or a list of such hashes)
# install the variables into the specified package,
# overwriting whatever variables were there before.
sub _install_hash {
  my $hashlist = shift;
  my $dest = shift;
  if (UNIVERSAL::isa($hashlist, 'HASH')) {
    $hashlist = [$hashlist];
  }
  my $hash;
  foreach $hash (@$hashlist) {
    my $name;
    foreach $name (keys %$hash) {
      my $val = $hash->{$name};
      no strict 'refs';
      local *SYM = *{"$ {dest}::$name"};
      if (! defined $val) {
	delete ${"$ {dest}::"}{$name};
      } elsif (ref $val) {
	*SYM = $val;
      } else {
 	*SYM = \$val;
      }
    }
  }
}

sub TTerror { $ERROR }

1;


=head1 NAME

Text::Template - Expand template text with embedded Perl

=head1 VERSION

This file documents C<Text::Template> version B<1.45>

=head2 Author

Mark-Jason Dominus, Plover Systems

Please send questions and other remarks about this software to
C<mjd-perl-template+@plover.com>

You can join a very low-volume (E<lt>10 messages per year) mailing
list for announcements about this package.  Send an empty note to
C<mjd-perl-template-request@plover.com> to join.

For updates, visit C<http://www.plover.com/~mjd/perl/Template/>.

=head2 Support?

This software is version 1.45.  It may have bugs.  Suggestions and bug
reports are always welcome.  Send them to
C<mjd-perl-template+@plover.com>.  (That is my address, not the address
of the mailing list.  The mailing list address is a secret.)

=head1 LICENSE

    Text::Template version 1.45
    Copyright (C) 2008 Mark Jason Dominus

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  You may also can
    redistribute it and/or modify it under the terms of the Perl
    Artistic License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received copies of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


=head1 THANKS

Many thanks to the following people for offering support,
encouragement, advice, bug reports, and all the other good stuff.

David H. Adler /
Joel Appelbaum /
Klaus Arnhold /
AntE<oacute>nio AragE<atilde>o /
Kevin Atteson /
Chris.Brezil /
Mike Brodhead /
Tom Brown /
Dr. Frank Bucolo /
Tim Bunce /
Juan E. Camacho /
Itamar Almeida de Carvalho /
Joseph Cheek /
Gene Damon /
San Deng /
Bob Dougherty /
Marek Grac /
Dan Franklin /
gary at dls.net /
Todd A. Green /
Donald L. Greer Jr. /
Michelangelo Grigni /
Zac Hansen /
Tom Henry /
Jarko Hietaniemi /
Matt X. Hunter /
Robert M. Ioffe /
Daniel LaLiberte /
Reuven M. Lerner /
Trip Lilley /
Yannis Livassof /
Val Luck /
Kevin Madsen /
David Marshall /
James Mastros /
Joel Meulenberg /
Jason Moore /
Sergey Myasnikov /
Chris Nandor /
Bek Oberin /
Steve Palincsar /
Ron Pero /
Hans Persson /
Sean Roehnelt /
Jonathan Roy /
Shabbir J. Safdar /
Jennifer D. St Clair /
Uwe Schneider /
Randal L. Schwartz /
Michael G Schwern /
Yonat Sharon /
Brian C. Shensky /
Niklas Skoglund /
Tom Snee /
Fred Steinberg /
Hans Stoop /
Michael J. Suzio /
Dennis Taylor /
James H. Thompson /
Shad Todd /
Lieven Tomme /
Lorenzo Valdettaro /
Larry Virden /
Andy Wardley /
Archie Warnock /
Chris Wesley /
Matt Womer /
Andrew G Wood /
Daini Xie /
Michaely Yeung

Special thanks to:

=over 2

=item Jonathan Roy

for telling me how to do the C<Safe> support (I spent two years
worrying about it, and then Jonathan pointed out that it was trivial.)

=item Ranjit Bhatnagar

for demanding less verbose fragments like they have in ASP, for
helping me figure out the Right Thing, and, especially, for talking me
out of adding any new syntax.  These discussions resulted in the
C<$OUT> feature.

=back

=head2 Bugs and Caveats

C<my> variables in C<fill_in> are still susceptible to being clobbered
by template evaluation.  They all begin with C<fi_>, so avoid those
names in your templates.

The line number information will be wrong if the template's lines are
not terminated by C<"\n">.  You should let me know if this is a
problem.  If you do, I will fix it.

The C<$OUT> variable has a special meaning in templates, so you cannot
use it as if it were a regular variable.

There are not quite enough tests in the test suite.

=cut
