#!/usr/bin/env python

import sys
import os
from os import path
try:
	import xml.etree.cElementTree as etree
except ImportError:
	import xml.etree.ElementTree as etree
import argparse
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('-s','--source', \
										help='Path to the xml defining body', \
										required=True)
parser.add_argument('-b','--bookinfo', \
										help='Path to the xml file contain bookinfo element')
parser.add_argument('-x','--xml', \
										help='Save the xml generated for the book', \
										type=bool,
										default=False)
parser.add_argument('-p','--pdf', \
										help='Generate the pdf', \
										type=bool,
										default=True)

class ChapterUpgrader:
	'''
	Reads in an xml tree from file and upgrades all
	first level sections to chapters. 
	'''
	tree = None
	def __init__(self,path):
		self.tree = etree.parse(path)
		root = self.tree.getroot()
		root.tag = 'book'
		self.sections_to_chapters(root)
	
	def sections_to_chapters(self,root):
		for element in root:
			if element.tag == 'section':
				element.tag = 'chapter'
	
	def get_tree(self):
		return self.tree
	
	def insert_bookinfo(self,path):
		infoRoot = etree.parse(path).getroot()
		root = self.tree.getroot()
		
		title = root.find('title')
		if title != None:
			root.remove(title)
		
		root.insert(0,infoRoot)

# Set up some useful names
args = parser.parse_args()

targetFile     = args.source
baseTargetFile = path.splitext(targetFile)[0]
upgradedFile   = path.join('.',baseTargetFile+'_upgr.xml')
outputFile     = path.join('.',baseTargetFile+'_autobook.xml')

bookInfoFile   = args.bookinfo

# upgrade chapter to book
upgradedTree = ChapterUpgrader(targetFile)

if bookInfoFile:
	upgradedTree.insert_bookinfo(bookInfoFile)

textTree = etree.tostring(upgradedTree.get_tree().getroot())

# insert the declaration
docbook_decl = '<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.3//EN" ' \
          			' "file:///usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">'

newfile = open(outputFile,'w+')
newfile.write('<?xml version="1.0" encoding="ASCII"?>\n')
newfile.write(docbook_decl+'\n')
newfile.write(textTree+'\n')

newfile.close()

# generate the pdf is desired
if args.pdf:
	subprocess.call(['docbook2pdf',outputFile])

# clean up that is requested
if not args.xml:
	os.remove(outputFile)
