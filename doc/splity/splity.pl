#!/usr/bin/perl

# (C) 2002 Yonat Sharon

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

use strict;

use Text::Template;

# documentation:
sub usage {
    print <<END;

splity.pl - Split a long HTML file to smaller HTML files
according to sections with <hN> headings.

Usage:
  split.pl [-h N] [-index <file>] [-page <file>] <html-file>
    <html-file>    source HTML file to split
    -h N           split on <h1> to <hN> (default is 3)
    -index <file>  template file to use for the index page
                   (default is index-template.html)
    -page <file>   template file to use for the section pages
                   (default is page-template.html)

See splity.html for details on the format of the source HTML
file and the template files.
END

    exit;
}

return 1 if ($0 =~ /splity.t$/);
usage() unless (@ARGV);

# init parameters:
my $source = pop; # TODO: default to 'all.html'
my %param = @ARGV;
my $max_h_level = $param{'-h'} || 3;
my $index_template = $param{'-index'} || 'index-template.html';
my $page_template  = $param{'-page'}  || 'page-template.html';

# extract data:
my $s = slurp_file($source);
my ($opening, $closing, @sections) = split_sections($s, $max_h_level);
my $doc_vars = {'opening' => $opening, 'closing' => $closing, 'source' => $source};
my @pages;
foreach my $section (@sections) {
    push(@pages, extract_page_data($section));
}
create_structure(\@pages);

# fill-in empty names and titles:
for (my $i = 0; $i < scalar(@pages); ++$i) {
    $pages[$i]->{'name'} ||= $i || 'index'; # TODO: should be according to parent, like 1.4.2
    $pages[$i]->{'title'} ||= $pages[$i]->{'name'};
}

# create index
my $index_page = shift @pages;
my $tt = new Text::Template(SOURCE => $index_template)
    or die "Can't construct template $index_template: $Text::Template::ERROR";
output_page($tt, $index_page, $doc_vars);

# create pages
$tt = new Text::Template(SOURCE => $page_template)
    or die "Can't construct template $page_template: $Text::Template::ERROR";
foreach my $page (@pages) {
    output_page($tt, $page, $doc_vars);
}

################################################################################

### output_page($template, $page) # Create output file for $page using $template.
sub output_page {
    my ($template, $page, $doc_vars) = @_;
    open(F, "> $page->{'name'}.html")
        or die "Can't open $page->{'name'}.html for writing: $!";
    $template->fill_in(HASH => [$page, $doc_vars], OUTPUT => \*F)
        or die "Can't fill_in template for $page->{'name'}: $Text::Template::ERROR";
    close F;
}

### $s = slurp_file($filename) # Read the whole file into a string. ############
sub slurp_file {
    local $/;
    open(F, $_[0]) or die "Can't open $_[0]: $!";
    my $s = <F>;
    close(F);
    return $s;
}

### ($opening, $closing, @sections) = split_sections($html, $max_h_level) ######
### Split HTML file to sections according to headings 1-$max_h_level.
sub split_sections { # split to @sections, and common $opening and $closing
    my $level = qr/[1-$_[1]]/i;
    my @sections = split(/(?=<h$level\W)/i, $_[0]);
    
    # cut $closing from last section
    my $closing;
    my $closing_tag = qr/<h\d[^>]+style="[^"]*display\s*:\s*none\s*;/is;
    if ($sections[$#sections] =~ /^$closing_tag/) {
        $closing = pop @sections;
    } else {
        $sections[$#sections] =~ s!($closing_tag.*)!!is;
        $sections[$#sections] =~ s!(</body.*)!!is unless ($1);
        $closing = $1;
    }
    return (shift @sections, $closing, @sections);
}

### $hash_ref = extract_page_data($html_section) ###############################
### Extract the following fields from an HTML page section:
###   title     - heading text
###   level     - heading level
###   name      - <a name> in heading
###   title_tag - complete heading tag
###   content   - everything but the heading
###   excerpt   - content of <excerpt></excerpt> tag (if exists)
sub extract_page_data {
    local $_ = $_[0];
    my $page;
    my $title;

    # extract <hN>title</hN>
    s!<(h(\d)[^>]*)>(.*?)</h\2>!!is;
    $page->{'title_tag'} = $1;
    $page->{'level'} = $2;
    $title = $3;

    # extract <a name="name">
    $page->{'name'} = $1 if
        $title =~ s!<a[^>]+name="(.*?)"[^>]*>(.*?)</a>!$2!is; # TODO: quotes should not be required
    $title =~ s!\s*\n! !g;
    $page->{'title'} = $title;

    # extract <excerpt>excerpt</excerpt>
    $page->{'excerpt'} = $1 if s!<excerpt>([^<]*)</excerpt>!$1!i;

    # redirect internal links
    s!\bhref="#(.*?)"!href="$1.html"!gi; # TODO: only inside A tags!

    $page->{'content'} = $_;

    return $page;
}

### create_structure(\@array_of_hash_refs) #####################################
### Create linking fields between @array elements, based on their 'level' field:
###   top  - first element
###   prev - previous element
###   next - next element
###   up   - parent element
###   sub  - array of child elements (direct children only)
sub create_structure {
    my $array = shift;
    my $top = $array->[0];
    my $level = $top->{'level'};
    my $last;
    foreach my $curr (@$array) {
        $curr->{'top'} = $top;
        if ($last) {
            $curr->{'prev'} = $last;
            $last->{'next'} = $curr;
            
            # find parent:
            my $curr_level = $curr->{'level'};
            my $last_level = $last->{'level'};
            if ($last_level == $curr_level) { # same level
                $curr->{'up'} = $last->{'up'};
            } elsif ($last_level < $curr_level) { # sub-section
                $curr->{'up'} = $last;
            } else { # up-section
                my $up = $last->{'up'};
                while ($up->{'level'} >= $curr_level) {
                    $up = $up->{'up'};
                }
                $curr->{'up'} = $up;
            }
            
            push(@{$curr->{'up'}->{'sub'}}, $curr);
        }
        $last = $curr;
    }
}

### $html = sub_list(\@sub, $depth, $show_excerpts, $item_tag, $sub_list_tag) ##
### Return HTML multi-level list of descendants.
###   \@sub - direct children
###   $depth - show descendants up to $depth level (0 means show all)
###   $excerpt_format - format string for excerpt, with '$1' as fill-in place.
###                     e.g., '<span class="excerpt">&quot;$1 ...&quot;</span>'
###   $item_tag - wrap each descendant with this tag (eg 'li')
###   $sub_list_tag - wrap descendants' sub-lists with this tag (eg 'ul')
sub sub_list {
    my $sub = shift;
    join("\n" , map {sub_list_item($_, @_)} @$sub);
}

sub sub_list_item {
    my ($page, $depth, $excerpt_format, $item_tag, $sub_list_tag) = @_;
    my $html = qq(<a href="$page->{'name'}.html">$page->{'title'}</a>);
    if ($excerpt_format and $page->{'excerpt'}) {
        $excerpt_format =~ s/\$1/$page->{'excerpt'}/;
        $html .= $excerpt_format;
    }
    if ($depth != 1 and $page->{'sub'}) {
        --$depth if ($depth > 0);
        $html .= "<$sub_list_tag>";
        $html .= sub_list($page->{'sub'}, $depth, $excerpt_format, $item_tag, $sub_list_tag);
        $html .= '</' . end_tag($sub_list_tag) . '>';
    }
    return "<$item_tag>$html</" . end_tag($item_tag) . '>';
}

### $closing_tag = end_tag($tag_with_attributes) ###############################
sub end_tag {
    my $tag = $_[0];
    $tag =~ s/\W.*//s;
    return $tag;
}

### $text = strip_tags($html) # Strip tags out of $html. #######################
sub strip_tags {
    local $_ = $_[0];
    s/<.*?>//gs;
    s/^\s*//s;
    s/\s*$//s;
    return $_;
}
