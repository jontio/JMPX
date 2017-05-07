#+= means use -= means dont use

CONFIG += use_build_dir
CONFIG += use_release_mode
CONFIG += print_user_info_msgs

DEFINES -= DEV_TAB


#-------------------------------

use_release_mode {
CONFIG -= debug release
CONFIG += release
}


