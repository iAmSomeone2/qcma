# Builds the vitamtp submodule as a static library with its own autotools build
TEMPLATE = aux

VITAMTP_SRC = $$PWD/../vitamtp

# Fedora keeps gettext's lib-link macros (needed by iconv.m4) outside aclocal's default path
exists(/usr/share/gettext/m4): VITAMTP_ACLOCAL_FLAGS = -I /usr/share/gettext/m4

# phony so that changes to the submodule are always picked up by its own make
vitamtp.target = vitamtp
vitamtp.CONFIG = phony
vitamtp.commands = \
    (test -x $$VITAMTP_SRC/configure || (cd $$VITAMTP_SRC && ACLOCAL_FLAGS=\"$$VITAMTP_ACLOCAL_FLAGS\" ./autogen.sh)) && \
    mkdir -p build && cd build && \
    (test -f config.status || $$VITAMTP_SRC/configure --disable-shared --enable-static --with-pic) && \
    $(MAKE)

first.depends = vitamtp
QMAKE_EXTRA_TARGETS += first vitamtp
