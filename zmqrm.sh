#!/bin/bash
git submodule deinit -f -- libzmq
rm -rf .git/modules/libzmq
git rm -f libzmq
git rm --cached libzmq
