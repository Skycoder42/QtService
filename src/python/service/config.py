#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import importlib

def main(command: str):
	if command == "suffix":
		print(importlib.machinery.EXTENSION_SUFFIXES[0])
	else:
		sys.exit(1)

if __name__ == '__main__':
	main(sys.argv[1])
