
# Uncomment following line to use fastest CFLAGS for gcc.
#   program will only run on hardware that is equal or better to
#   the current host.
#CFLAGS ?= -O3 -march=native -mtune=native -flto -fuse-linker-plugin

# Default lame CFLAGS.
CFLAGS ?= -O3 -flto -fuse-linker-plugin

LDFLAGS ?= -flto -fuse-linker-plugin

C_STANDARD=$(shell test -f .use_c_std && cat .use_c_std || (echo 'int main(int argc, char *argv[]) { return 0; }' > .uc.c; ${CC} -std=gnu99 .uc.c >/dev/null 2>&1 && (echo 'gnu99' > .use_c_std; echo 'gnu99'; rm -f .uc.c) || ( echo 'c99' > .use_c_std; echo 'c99'; rm -f .uc.c ) ))

# Actual flags to use.
USE_CFLAGS = ${CFLAGS} -Wall -Wno-unused-function -pipe -std=${C_STANDARD}

USE_LDFLAGS = -Wl,-O1,--sort-common,--as-needed,-z,relro ${LDFLAGS}

# Cause everything to recompile when CFLAGS changes, unless user is root (to support "sudo make install")
WHOAMI=$(shell whoami)
CFLAGS_HASH=$(shell echo "CFLAGS=${USE_CFLAGS} .. LDFLAGS=${USE_LDFLAGS}" | md5sum | tr ' ' '\n' | head -n1)
CFLAGS_HASH_FILE=$(shell test "${WHOAMI}" != "root" && echo .cflags.${CFLAGS_HASH} || echo .cflags.*)

PREFIX ?= $(shell test -w "/usr/bin" && echo "/usr" || echo "${HOME}")

DESTDIR ?= ${PREFIX}

DEPS = bin/.created ${CFLAGS_HASH_FILE} mtime_utils.h

ALL_FILES = bin/sort_mtime


all: ${DEPS} ${ALL_FILES}
#	@ /bin/true

clean:
	rm -Rf bin
	rm -f *.o
	rm -f .cflags.*

install: ${ALL_FILES}
	mkdir -p "${DESTDIR}/bin"
	install -m 775 ${ALL_FILES} "${DESTDIR}/bin"


# When hash of CFLAGS changes, this unit causes all compiles to become invalidated
${CFLAGS_HASH_FILE}:
	test "${WHOAMI}" != "root" -a ! -e "${CFLAGS_HASH_FILE}" && rm -f .cflags.* || true
	touch "${CFLAGS_HASH_FILE}"


static:
	make clean;
	CFLAGS="${CFLAGS} -static" make


bin/.created:
	mkdir -p bin
	touch bin/.created

sort_mtime.o : ${DEPS} sort_mtime.c
	gcc ${USE_CFLAGS} sort_mtime.c -c -o sort_mtime.o

bin/sort_mtime: ${DEPS} sort_mtime.o
	gcc ${USE_CFLAGS} ${USE_LDFLAGS} sort_mtime.o -o bin/sort_mtime

remake:
	make clean
	make all
