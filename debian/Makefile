#!/usr/bin/make -f

# This variable should be set by jenkins as an env variable, set a default value
BUILD_NUMBER ?= 0

# Parse debian/changelog (file with strict formatting/rules, reasonable to parse)
# 1) All non-space characters from the beginning of the first line
NAME_PKG=$(shell head -n1 changelog | sed '/^\([^ ]*\).*/!d;s//\1/')
# 2) All characters matching [.+~A-Za-z0-9] inside parentheses in the first line
# (https://github.com/SquadroneSystem/sqdr-debian/blob/main/deb-version.md)
VERSION=$(shell head -n1 changelog | sed '/.*(\([.+~A-Za-z0-9]*\).*).*/!d;s//\1/')

GIT_COMMIT=$(shell git rev-parse --short HEAD)
SQDR_DEBIAN_REVISION=jenkins$(BUILD_NUMBER)+git$(GIT_COMMIT)
VERSION_TRACEABLE=$(VERSION)-$(SQDR_DEBIAN_REVISION)
NAME_PKG_WITH_VERSION_TRACEABLE=$(NAME_PKG)_$(VERSION_TRACEABLE)

DIRECTORY_WITH_SOURCES=$(NAME_PKG)_$(VERSION)
UPSTREAM_TARBALL_WITH_SOURCES=$(DIRECTORY_WITH_SOURCES).orig.tar.gz

##############################
# User "public interface"
##############################

# Default target
.PHONY: all
all: clean deb

# Generic actions
.PHONY: clean
clean: _clean_package_version_dir
	-rm -rf *.deb *.ddeb *.tar.xz *.tar.gz *.buildinfo *.build *.changes *.dsc *.zip *.upload

# Packages
.PHONY: deb
deb: clean _deb_stage_1_env_independent _deb_stage_2_env_dependent

.PHONY: tar
tar: clean $(NAME_PKG_WITH_VERSION_TRACEABLE).tar.gz

.PHONY: zip
zip: clean $(NAME_PKG_WITH_VERSION_TRACEABLE).zip

# Actions on packages
.PHONY: dput
dput: deb /etc/sqdr/dput.cf
	find . -name '*.changes' -exec dput --config /etc/sqdr/dput.cf --force --unchecked sqdr-default {} \;

##############################
# User "private interface"
##############################

.PHONY: _clean_package_version_dir
_clean_package_version_dir:
	-find . -depth -name "$(NAME_PKG)_*" -type d -exec rm -rf {} \;

# NOTE! This "private interface" _deb_stage_1_env_independent target is called by jenkins, rename it at your own risk
.PHONY: _deb_stage_1_env_independent
_deb_stage_1_env_independent: $(DIRECTORY_WITH_SOURCES) $(UPSTREAM_TARBALL_WITH_SOURCES)

# NOTE! This "private interface" _deb_stage_2_env_dependent target is called by jenkins, rename it at your own risk
.PHONY: _deb_stage_2_env_dependent

DISTRIB_BUILD=$(shell lsb_release -cs)

_deb_stage_2_env_dependent: changelog compat control rules
	find ./$(DIRECTORY_WITH_SOURCES) -name 'changelog' -exec sed -i "1 s/^\(\S* ([^)]*)\) \S*;/\\1 ${DISTRIB_BUILD};/g" {} \;
	cd ./$(DIRECTORY_WITH_SOURCES) && \
	debuild \
		--preserve-envvar CCACHE_* \
		--unsigned-source -us \
		--unsigned-changes -uc \
		--build=binary -b \
		--changes-option=-DDistribution=${DISTRIB_BUILD}

.PHONY: _prepare_dir_for_release_with_sources
_prepare_dir_for_release_with_sources: clean $(DIRECTORY_WITH_SOURCES)
	@echo "\n\n================= WARNING! The resulting 'package' will contain the sources =================\n\n"
	@echo "\n=> Copy the changelog to the root of the directory to be released (useful for any project)"
	cp changelog $(DIRECTORY_WITH_SOURCES)/
	@echo "\n=> Delete all the other debian related files (not needed/useful)"
	rm -r $(DIRECTORY_WITH_SOURCES)/debian

##############################
# Non-phony targets!
##############################

SOURCE_DIR=$(shell dirname $(shell pwd))
$(DIRECTORY_WITH_SOURCES): changelog
	@echo "\n=> Copying source directory (${SOURCE_DIR}) to /tmp to build tarball"
	cp -r $(SOURCE_DIR) /tmp/$@
	@echo "\n=> Deleting git repositories and files from source directory to be tarballed (while in /tmp to avoid accidents)"
	-find /tmp/$@ -depth -name '.git*' -exec rm -rf {} \;
	@echo "\n=> Moving directory to be tarballed to current directory"
	mv /tmp/$@ .
	@echo "\n=> Clean the directory to be tarballed"
	-cd $@ && make clean
	@echo "\n=> Replacing the version to contain the jenkins job and git commit numbers"
	@echo " (https://github.com/SquadroneSystem/sqdr-debian/blob/main/deb-version.md)"
	find $@ -name 'changelog' -exec sed -i "1 s/(.*)/(${VERSION_TRACEABLE})/g" {} \;

$(UPSTREAM_TARBALL_WITH_SOURCES): $(DIRECTORY_WITH_SOURCES)
	@echo "\n=> Build/create the tarball"
	tar -zcvf $@ $(DIRECTORY_WITH_SOURCES)

$(NAME_PKG_WITH_VERSION_TRACEABLE).tar.gz: $(DIRECTORY_WITH_SOURCES) _prepare_dir_for_release_with_sources
	tar -zcvf $@ $(DIRECTORY_WITH_SOURCES)

$(NAME_PKG_WITH_VERSION_TRACEABLE).zip: $(DIRECTORY_WITH_SOURCES) _prepare_dir_for_release_with_sources
	zip -r $@ $(DIRECTORY_WITH_SOURCES)
