#!/usr/bin/env python
# -*- coding: utf-8 -*-
# encoding: utf-8

import re
import sys
import argparse
import subprocess
# import the template
from template import *

DEBUG = False


class  Option():

    def parse(self):
		# FIXME: sort the options
        argParser = argparse.ArgumentParser(description='''Wraps SIOX function
around library function calls''')

        argParser.add_argument('--blank-header', '-b', action='store_true',
                default=False, dest='blankHeader')

        argParser.add_argument('--output', '-o', action='store', nargs=1,
				dest='outputFile', default='out.h', help='out')

        argParser.add_argument('--debug', '-d', action='store_true', default=False,
				dest='debug', help='''Print out debug information.''')

        argParser.add_argument('--cpp', '-c', action='store_true', default=False,
				dest='cpp', help='Use cpp to pre process the input file.')

        argParser.add_argument('--altternative-var-names', '-n',
				action='store_true', default=False, dest='alternativeVarNames',
				help='''If a header file dose not provide variable names, this will
enumerate them.''')

        argParser.add_argument('--cppArgs', '-a', action='store',nargs=1, default=[],
				dest='cppArgs', help='''Pass arguments to the cpp. If this option is
specified the --cpp option is specified implied.''')

        argParser.add_argument('inputFile', default=None,
        help='Source or header file to parse')

        args = argParser.parse_args()

        if args.outputFile: args.outputFile = args.outputFile[0]
        if args.cppArgs: args.cpp = True

        return args

class Function():

    def __init__(self):

		# Return type of the function (int*, char, etc.)
        self.type= ''
        self.name = ''
		# A list of Parameters, a parameter is a extra class for storing
        self.parameters = []
        self.signature = ''
        self.usedTemplates = []
        self.special = ''
    def generateSignature(self):

			for sigpart in self.parameters:
				sig = sigpart.type + '' + sigpart.name + ', '
				self.signature += sig
			self.signature = self.signature[:-2]


    def getIdentyfier(self):

		params = ''

		for param in self.parameters:
			params += '%s,' % (param.__str__())

		params = params[:-1]

		identyfier = '%s%s(%s);' % (''.join(self.type.split(' ')),
			self.name.strip(), params)

		return identyfier
##
# @brief One parameter of a function.
class Parameter():

	def __init__(self):
		self.type = ''
		self.name = ''
	#end def


	def __str__(self):

		return ''.join(self.type.split(' '))+self.name
#end class

class Instruction():

	def __init__(self):
		self.name = ''
		self.parameters = []
	#end def
#end class

##
# @brief This class parses the abstract syntax tree generated by the pycparser
class FunctionParser():

    ##
    # @brief
    #
    # @param options
    #
    # @return
    def __init__(self, options):

        self.inputFile = options.inputFile
        self.cpp = options.cpp
        self.cppArgs = options.cppArgs
        self.alternativeVarNames = options.alternativeVarNames
        self.functions = []
        # This regex searches from the beginning of the file or } or ; to the next
        # ; or {.
        self.regexFuncDef = re.compile('(?:;|})?(.+?)(?:;|{)', re.M | re.S)

        # Filter lines with comments beginng with / or # and key word that looks
        # like function definitions
        self.regexFilter = re.compile('^[#/]|typedef.*')

        # Split every line that is left in function name and parameters.
        self.regexFuncDefParts = re.compile( '(.+?)\((.+?)\).*')

    ##
    # @brief
    #
    # @return
    def parse(self):

        functions = []
        lines = ''

        if self.cpp:
            cppCommand = ['cpp']
            cppCommand.extend(self.cppArgs)
            cppCommand.append(self.inputFile)
            cpp = subprocess.Popen(cppCommand,
                    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            lines, error = cpp.communicate()

        else:
            #FIXME
            inputFile = open(self.inputFile, 'r')
            lines = inputFile.read()

        matchFuncDef = self.regexFuncDef.findall(lines)

        for funcNo, function in enumerate(matchFuncDef):


            function = function.split('\n')

            for index in range(0,len(function)):
                function[index] = function[index].lstrip()

                if self.regexFilter.match(function[index]):
                    function[index] = ''

                function[index] = function[index].rstrip()

            function = ''.join(function)

            funcParts = self.regexFuncDefParts.match(function)

            if funcParts is None:
                continue

            function = Function()
            functions.append(function)
            function.type, function.name = self.getTypeName(funcParts.group(1),
            'function'+str(funcNo))

            parameters = funcParts.group(2).split(',')

            for paramNo, parameter in enumerate(parameters):
                parameterObj = Parameter()
                function.parameters.append(parameterObj)
                parameterObj.type, parameterObj.name = self.getTypeName(parameter, 'var'+str(paramNo))

            function.generateSignature()
        return functions

    ##
    # @brief
    #
    # @param declaration
    # @param alternativeVarName
    #
    # @return
    def getTypeName(self, declaration, alternativeVarName):

        type = ''
        name = ''

        parameterParts = declaration.split('*')

        if self.alternativeVarNames:
            if(len(parameterParts) == 1):
                type = parameterParts[0]
                name = alternativeName

            else:
                for element in parameterParts:
                    if element is '':
                        type += '*'

                    else:
                        type += element
                        name = alternativeName
                        type += '*'

        else:

            if(len(parameterParts) == 1):
                parameterParts = declaration.split()
                name = parameterParts.pop()

                for element in parameterParts:
                    type += element

            else:
                name = parameterParts.pop()

            for element in parameterParts:
                if element is '':
                    type+= '*'

                else:
                    type+= element
                    type += '*'

        return (type, name)

class CommandParser():

    def __init__(self,options):
        self.inputFile = options.inputFile

        self.commandRegex = re.compile('^\s*//\s*([-_.\w\d]+)\s*(.*)')
    def parse(self, functions):
        avalibalCommands  = template.keys()
        input = open(self.inputFile, 'r')
        inputLines = input.readlines()
        index = 0
        commandName=''
        commandArgs=''
        templateList = []
        for line in inputLines:
            match = self.commandRegex.match(line)
            if (match):
                if match.group(1) in avalibalCommands:
                    if commandName is not '':
                        templateList.append(templateClass(commandName, commandArgs))
                        commandName = ''
                        commandArgs = ''

                    commandName +=  match.group(1)
                    commandArgs += match.group(2)
                else:
                    commandArgs = match.group(1)
                    commandArgs = match.group(2)
            else:
                if re.sub('\s', '', line) == functions[index].getIdentyfier():
                    templateList.append(templateClass(commandName, commandArgs))
                    functions[index].usedTemplates = templateList
                    commandName = ''
                    commandArgs = ''
                    index+=1
        return functions
# This class is like SkyNet, but more advanced
class templateClass():
	def __init__(self, name, variables):
		templateDict = template[name]
		self.name = name
		self.parameters = {}
		self.setParameters(templateDict['variables'], variables)
		self.world = templateDict['global']
		self.init = templateDict['init']
		self.before = templateDict['before']
		self.after = templateDict['after']
		self.final = templateDict['final']

	# fill the parameter-lists
	def setParameters(self, names, values):
		NameList = names.split(' ')
		ValueList = values.split(' ')

		position = 0;

		# only iterate over the first elements
		for value in ValueList[0:len(NameList)-1]:
			self.parameters[NameList[position]] = value
			position += 1

		# all other elements belong to the last parameter
		self.parameters[NameList[position]] = ' '.join(ValueList[len(NameList)-1:])

	# puts everything together
	def output(self, type):
		if (type == 'global'):
			return self.world % self.parameters
		elif (type == 'init'):
			return self.init % self.parameters
		elif (type == 'before'):
			return self.before % self.parameters
		elif (type == 'after'):
			return self.after % self.parameters
		elif (type == 'final'):
			return self.final % self.parameters
		else:
			# Error
			# FIXME: Proper Error-Handling ;)
			return 'Einmal mit Profis arbeiten...'

# @brief
class Writer():

	##
	# @brief
	#
	# @param options
	#
	def __init__(self, options):

		self.outputFile = options.outputFile

	##
	# @brief
	#
	# @param functions
	#
	# @return
	def headerFile(self, functions):
		# open the output file for writing
		file = open(self.outputFile, 'w')

		# function headers
		for function in functions:
			file.write("%s %s ( %s ); \n" % (function.type,
				function.name, function.signature))

		# close the file
		file.close()

	def sourceFile(self, functions):
		# open the output file for writing
		file = open(self.outputFile, 'w')

		# generic includes
		file.write('#include <stdio.h> \n#include <stdlib.h>')

		# global stuff
		for function in functions:
			for temp in function.usedTemplates:
				file.write('%s\n' % (temp.output('global')))

		# redefinition
		for function in functions:
			file.write('%s *__real_%s(%s);\n' % (function.type, function.name, function.signature))

		# functions
		for function in functions:
			# is this the desired init-function?
			if function.special == 'init':
				# write all init-functions
				for func in functions:
					for temp in func.usedTemplates:
						file.write('\t%s\n' % (temp.output('init')))

			# is this the desired final-function?
			if function.special == 'final':
				# write all init-functions
				for func in functions:
					for temp in func.usedTemplates:
						file.write('\t%s\n' % (temp.output('final')))

			file.write('%s *__wrap_%s(%s)\n{\n' % (function.type, function.name, function.signature))

			for temp in function.usedTemplates:
				file.write('\t%s\n' % (temp.output('before')))

			file.write('\treturn __real_%s(%s);\n' % (function.name, function.signature))

			for temp in function.usedTemplates:
				file.write('\t%s\n' % (temp.output('after')))

			file.write('}\n\n');

##
# @brief The main function.
#
def main():

	# Parse input parameter.
    opt = Option()
    options = opt.parse()


    functionParser = FunctionParser(options)
    outputWriter = Writer(options)

    functions = functionParser.parse()

    if options.blankHeader:
        outputWriter.headerFile(functions)

    else:
        commandParser = CommandParser(options)
        functions = commandParser.parse(functions)
        print functions[0].usedTemplates
        outputWriter.sourceFile(functions)

if __name__  == '__main__':
	main()


