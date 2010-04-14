# export EXTRA_ARGS="--project-mode"
# export GTK2_RC_FILES="$GTK2_RC_FILES:$bundle_etc/gtkrc.medit"

# ige-mac-bundler sets it to some garbage, unset it
unset LANG

export PYTHONHOME="$bundle_res"
export PATH="$bundle_bin:$PATH"

export BUNDLE_CONTENTS="$bundle_contents"
