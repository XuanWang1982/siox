#!/usr/bin/env python
# -*- coding: utf-8 -*-
# encoding: utf-8
from __future__ import print_function

import re
import sys
import argparse
import subprocess
# import the template
from template import *

DEBUG = False


##
# @brief Generate and handle the command line parsing.
#
# This class is just a simple Wrapper of the python argparser.
class  Option():

    ##
    # @brief Parse the command line arguments.
    #
    # Parses the command line arguments, using the python argparser.
    def parse(self):
        prog = 'SIOX-Wrapper'

        description = '''The SIOX-Wrapper is a tool which instrument software
libraries, by calling trace functions before and after the actual library call.'''

        argParser = argparse.ArgumentParser(description=description, prog=prog)

        argParser.add_argument('--output', '-o', action='store', nargs=1,
        dest='outputFile', default=['out'], help='Provide a file to write the output.')

        argParser.add_argument('--debug', '-d', action='store_true', default=False,
        dest='debug', help='''Print out debug information.''')

        argParser.add_argument('--blank-header', '-b', action='store_true',
                default=False, dest='blankHeader',
                help='''Generate a clean header file which can be instrumented
from a other header file or C source file.''')

        argParser.add_argument('--cpp', '-c', action='store_true', default=False,
        dest='cpp', help='Use cpp to preprocess the input file.')

        argParser.add_argument('--cpp-args', '-a', action='store', nargs=1, default=[],
        dest='cppArgs', help='''Pass arguments to the cpp. If this option is
specified the --cpp option is specified implied.''')

        argParser.add_argument('--alt-var-names', '-n',
        action='store_true', default=False, dest='alternativeVarNames',
        help='''If a header file dose not provide variable names, this will
enumerate them.''')

        argParser.add_argument('--style', '-s',
        action='store', default='wrap', dest='style',
        choices=['wrap', 'dllsym'],
        help='''Designates which output-style to use.''')

        argParser.add_argument('inputFile', default=None,
        help='Source or header file to parse.')

        args = argParser.parse_args()

        if args.outputFile:
            args.outputFile = args.outputFile[0]
        if args.cppArgs:
            args.cpp = True

        return args


##
# @brief A storage class for a function.
#
# Stores a Function and offers functions for formatted output.
class Function():

    def __init__(self):

        ## Return type of the function (int*, char, etc.)
        self.type = ''
        ## Name of the function
        self.name = ''
        ## A list of Parameters, a parameter is a extra class for storing.
        self.parameters = []
        ## A list of templates associated with the function.
        self.definition = ''
        self.usedTemplates = []
        ## Indicates that the function is the first called function of the
        # library and initialize SIOX.
        self.init = False
        ## Indicates that the function is the last called function of the
        # library.
        self.final = False

    ##
    # @brief Generate the function call.
    #
    # @param self The reference to this object.
    #
    # @return A string containing the function call.
    #
    # Generates the function call of the function.
    def getCall(self):

        if len(self.parameters) == 1:
            if self.parameters[0].name == 'void':
                return '%s()' % (self.name)
        return '%s(%s)' % (self.name,
         ', '.join(paramName.name for paramName in self.parameters))

    ##
    # @brief Generate the function call for a function pointer for dlsym.
    #
    # @param self The reference to this object.
    #
    # @return A string containing the function call for a function pointer
    def getPointerCall(self):

        if len(self.parameters) == 1:
            if self.parameters[0].name == 'void':
                return '(*%s)()' % (self.name)
        return '(* __real_%s)(%s)' % (self.name,
            ', '.join(paramName.name for paramName in self.parameters))

    ##
    # @brief Generate the function definition.
    #
    # @param self The reference to this object.
    #
    # @return The function definition as a string without the type.
    def getDefinition(self):

        if self.definition == '':

            return '%s(%s)' % (self.name,
                ', '.join('  '.join([param.type, param.name])
                for param in self.parameters))

        else:
            return '%s %s %s' % (self.type, self.name, self.definition)

    def getPointerDefinition():

        if self.definition == '':

            parameters = ', '.join(' '.join([parameter.type, parameter.name])
                for parameter in paramters)

            return ('%s (*__real_%s) (%s) = (%s (*) (%s)) dlsym(dllib, \
                (const char*) "%s");' % (self.type, self.name, paramters,
                self.type, parameters, self.name)

        else:

            return ('%s (*__real_%s) %s = (%s (*) %s) dlsym(dllib, \
                (const char*) "%s");' % (self.type, self.name, self.definition,
                self.type, self.definition, self.name)

    ##
    # @brief Generate an identifier of the function.
    #
    # The generated identifier is used as a key for the hash table the functions
    # are stored in.
    #
    # @return A unique identifier for the function as a string.
    def getIdentifier(self):

        identifier = '%s%s(%s);' % (self.type, self.name,
                ''.join(''.join([param.type, param.name]) for param in self.parameters))

        return re.sub('[,\s]', '', identifier)


##
# @brief One parameter of a function.
#
# This class holds only data.
class Parameter():

    def __init__(self):
        ## The type of the parameter.
        self.type = ''
        ## The name of the parameter.
        self.name = ''


##
# @brief An instruction form an instrumented header file.
#
# The instructions a read form a instrumented header file. The instruction
# names are the entries found in the template file.
class Instruction():

    def __init__(self):
        ## Name of the instruction.
        self.name = ''
        ## Parameters of the instruction.
        self.parameters = []


##
# @brief Parses a header file and looks for function definitions.
#
# The function parser searches through a header file using regular expressions.
class FunctionParser():

    def __init__(self, options):

        self.inputFile = options.inputFile
        self.cpp = options.cpp
        self.cppArgs = options.cppArgs
        self.blankHeader = options.blankHeader

        ## This regular expression searches for the general function definition.
        # ([\w*\s]) matches the return type of the function.
        # (?:\s*\w+\s*\()) is a look ahead that ensures that the first part of
        # the ends right before the function name. The look ahead searches for
        # last 'word'right in front of the (.
        # the \s*(\w+)\s* actually matches the function name (a look ahead
        # doesn't consume text).
        # \(([,\w*\s\[\]]*)\) matches the parentheses of the function definition
        # and groups everything inside them. The regex can't match single
        # parameters because a regex must have a fixed number of groups to match.
        self.regexFuncDef = re.compile('(?:([\w*\s]+?)(?=\s*\w+\s*\())\s*(\w+)\s*\(([,\w*\s\[\]]*)\)[\w+]*?;',
        re.S | re.M)

        ## This regular expression matches parameter type and name.
        # The Parameter which is matched needs to have a type and a name and is
        # only used when the C code of the instrumented header file is
        # generated. The instrumented header must provide the type and the name
        # of every parameter.
        # [\w*\s]+ matches everything until the last * or space.
        # (?:\*\s*|\s+) matches the last * or blank
        # ([\w]+ matches the parameter name
        # (?:\s*\[\s*\])? matches array [] if exist
        self.regexParamterDef = re.compile('([\w*\s]+(?:\*\s*|\s+))([\w]+(?:\s*\[\s*\])?)')

        ## This tuple of filter words searches for reseverd words in the
        ## function return type.
        # Some typedefs can look like function a definition the regex.
        self.Filter = ('typedef')

    ##
    # @brief This function parses the header file.
    #
    # @return A list of function definitions.
    #
    # @param self The reference to the object.
    def parse(self, string):

        functions = []

        # Match all function definitions
        iterFuncDef = self.regexFuncDef.finditer(string)

        for funcDef in iterFuncDef:

            # filter typedefs
            if funcDef.group(1).find(self.Filter) != -1:
                continue

            # At this point we are quite certain, we found a function definition
            function = Function()

            # Extract the type an name form the match object.
            function.type = funcDef.group(1).strip()
            function.name = funcDef.group(2).strip()

            # Get the parameter string.
            parameterString = funcDef.group(3).strip()
            # Substitute newlines and multiple blanks
            parameterString = re.sub('\n', '', parameterString)
            parameterString = re.sub('\s+', ' ', parameterString)

            # If only the blank header file should be generated pass the
            # string of parameters to the Function object.
            if self.blankHeader:

                    function.definition = "(" + parameterString + ")"

            # If the C source code should be generated split the string into a
            # list of parameters and extract the type and name of the parameter.
            else:
                parameterList = parameterString.split(',')

                for parameter in parameterList:
                    parameterObject = Parameter()
                    parameter = parameter.strip()

                    if parameter in ('void', '...'):
                        parameterName = ''
                        parameterType = parameter

                    else:

                        parameterMatch = self.regexParamterDef.match(parameter)
                        parameterType = parameterMatch.group(1)
                        parameterName = parameterMatch.group(2)

                        # Search for something like 'int list[]' and convert it to
                        # 'int* list'
                        regexBracketes = re.compile('\[\s*\]')
                        if regexBracketes.search(parameterName):
                            parameterName = regexBracketes.sub('', parameterName)
                            parameterType += '*'

                    parameterObject.name = parameterName
                    parameterObject.type = parameterType

                    function.parameters.append(parameterObject)

            functions.append(function)

        return functions

    ## @brief Wrapper function for parse().
    #
    # @return A list of function objects
    #
    # @param self
    #
    # This is a wrapper function for the parse() function for parsing the
    # passed to the FunctionParser object with the option argument.
    def parseFile(self):

        # Call the cpp.
        if self.cpp:
            cppCommand = ['cpp']
            cppCommand.extend(self.cppArgs)
            cppCommand.append(self.inputFile)
            cpp = subprocess.Popen(cppCommand,
                    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            string, error = cpp.communicate()

            if error != '':
                print('ERROR: CPP error:\n', error, file=sys.stderr)
                sys.exit(1)

        else:
            inputFile = open(self.inputFile, 'r')
            string = inputFile.read()

        functions = self.parse(string)
        return functions

    # @brief Wrapper function for parse()
    #
    # @return A list of function objects
    #
    # @param self
    #
    # param string
    def parseString(self, string):

        functions = self.parse(string)
        return functions


##
# @brief The command parser which reads the instrumentation instructions.
#
# The command parser reads the instrumentation form a commented header file.
# The parsing is based on regular expressions.
class CommandParser():

    def __init__(self, options):
        self.inputFile = options.inputFile
        self.options = options
        ## This regular expression matches the instructions which begin with //
        self.commandRegex = re.compile('^\s*//\s*(.+?)\s+(.*)')

    ##
    # @brief This function parses the header file.
    #
    # @param self The reference to the object.
    # @param functions The list of function objects returned by the function
    # parser.
    #
    #
    # @return A list of function objects which are extended by the
    # instrumentation instructions.
    def parse(self, functions):

        functionParser = FunctionParser(self.options)

        avalibalCommands = template.keys()
        input = open(self.inputFile, 'r')
        inputLines = input.readlines()
        index = 0
        commandName = ''
        commandArgs = ''
        templateList = []
        currentFunction = ''

        # Iterate over every line and search for instrumentation instructions.
        for line in inputLines:

            currentFunction = functionParser.parseString(line)
            if len(currentFunction) == 1:
                currentFunction = currentFunction[0]
            else:
                currentFunction = Function()

            match = self.commandRegex.match(line)
            if (match):
                # Search for init or final sections to define init or final
                # functions
                if match.group(1) == 'init':
                    functions[index].init = True

                elif match.group(1) == 'final':
                    functions[index].final = True

                elif match.group(1) in avalibalCommands:
                    # If a new command name is found and a old one is still
                    # used, save the old command and begin a new instruction
                    if commandName != '':
                        templateList.append(templateClass(commandName, commandArgs))
                        commandName = ''
                        commandArgs = ''

                    commandName += match.group(1)
                    commandArgs += match.group(2)

                else:
                    # If no command name is defined and a comment which is no
                    # instruction is found throw an error.
                    if commandName == '':
                        print("ERROR: Command not known: ", match.group(1), file=sys.stderr)
                        sys.exit(1)

                    commandArgs = match.group(1)
                    commandArgs = match.group(2)

            else:
                #If a function is found append the found instructions to the function object.
                if currentFunction.getIdentifier() == functions[index].getIdentifier():
                    if commandName != '':
                        templateList.append(templateClass(commandName, commandArgs))
                        functions[index].usedTemplates = templateList
                        templateList = []
                        commandName = ''
                        commandArgs = ''
                    index = index + 1

        return functions


##
# @brief Used to store the templates for each function
class templateClass():
    ##
    # @brief The constructor
    #
    # @param name The name of the used template
    # @param variables The used variables
    def __init__(self, name, variables):
        templateDict = template[name]
        self.name = name
        self.parameters = {}

        # Generate strings for output from given input
        self.setParameters(templateDict['variables'], variables)

        # Remember template-access for easier usage
        self.world = templateDict['global']
        self.init = templateDict['init']
        self.before = templateDict['before']
        self.after = templateDict['after']
        self.final = templateDict['final']

    ##
    # @brief Reads the parameters and generates the needed output
    #
    # @param names The name of the used template
    # @param values The used variables
    def setParameters(self, names, values):
        # generate a list of all names and values
        NameList = names.split(' ')
        ValueList = values.split(' ')

        position = 0
        # iterate over all elements but the last one
        for value in ValueList[0:len(NameList) - 1]:
            self.parameters[NameList[position]] = value
            position += 1
        # all other elements belong to the last parameter
        self.parameters[NameList[position]] = ' '.join(ValueList[len(NameList) - 1:])

    ##
    # @brief Used for selective output
    #
    # @param type What part of the template should be given
    #
    # @return The requested string from the template
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
            print('ERROR: Section: ', type, ' not known.', file=sys.stderr)
            sys.exit(1)


##
# @brief The output class (write a file to disk)
#
class Writer():

    ##
    # @brief The constructor
    #
    # @param options The supplied arguments
    def __init__(self, options):
        self.outputFile = options.outputFile

    ##
    # @brief Write a header file
    #
    # @param functions A list of function-objects to write
    def headerFile(self, functions):
        # open the output file for writing
        output = open(self.outputFile, 'w')

        # write all function headers
        for function in functions:
            for match in throwaway:
                if re.search(match, function.getDefinition()):
                    function.name = ''

            if function.type == '':
                continue

            elif function.name == '':
                continue

            else:
                print(function.getDefinition(), end=';\n', sep='', file=output)

        # close the file
        output.close()

    ##
    # @brief Write a source file
    #
    # @param functions A list of function-objects to write
    def sourceFileWrap(self, functions):
        # open the output file for writing
        output = open(self.outputFile, 'w')

        # write all needed includes
        for match in includes:
            print('#include ', match, end='\n', file=output)
        print('\n', file=output)

        # write all global-Templates
        for func in functions:
            for templ in func.usedTemplates:
                if templ.output('global') != '':
                    print(templ.output('global'), file=output)
        print("", file=output)

        # write all function redefinitions
        for function in functions:
            print(function.type, ' __real_', function.getDefinition(),
                    end=';\n', sep='', file=output)

        print("", file=output)

        # write all functions-bodies
        for function in functions:
            # write function signature

            print(function.type, ' __wrap_', function.getDefinition(),
                    end='\n{\n', sep='', file=output)

            # a variable to save the return-value
            returntype = function.type

            if returntype != "void":
                print('\t', returntype, ' ret;', end='\n', sep='',
                        file=output)

            # is this the desired init-function?
            if function.init:
                # write all init-templates
                for func in functions:
                    for temp in func.usedTemplates:
                        outstr = temp.output('init').strip()
                        if outstr.strip() != '':
                            print('\t', outstr, end='\n', sep='', file=output)

            # write the before-template for this function
            for temp in function.usedTemplates:
                outstr = temp.output('before').strip()
                if outstr != '':
                    print('\t', outstr, end='\n', sep='', file=output)

            # write the function call
            if returntype != "void":
                print('\tret = __real_', function.getCall(), end=';\n', sep='',
                        file=output)
            else:
                print('\t__real_', function.getCall(), end=';\n', sep='',
                        file=output)

            # is this the desired final-function?
            if function.final:
                # write all final-functions
                for func in functions:
                    for temp in func.usedTemplates:
                        outstr = temp.output('final').strip()
                        if outstr.strip() != '':
                            print('\t', outstr, end='\n', sep='', file=output)

            # write all after-templates for this function
            for temp in function.usedTemplates:
                outstr = temp.output('after').strip()
                if outstr != '':
                    print('\t', outstr, end='\n', sep='', file=output)

            # write the return statement and close the function
            if returntype != "void":
                print('\treturn ret;\n}', end='\n\n', file=output)

            else:
                print('\n}', end='\n\n', file=output)

        # generate gcc string for the user
        gcchelper = '-Wl'

        for func in functions:
            gcchelper = "%s,--wrap=\"%s\"" % (gcchelper, func.name)

        print(gcchelper)

        # close the file
        output.close()

    ##
    # @brief Write a source file
    #
    # @param functions A list of function-objects to write
    def sourceFileDLSym(self, functions):
        # open the output file for writing
        output = open(self.outputFile, 'w')

        print('#include <dlfcn.h>\nvoid* dllib = dlopen(', dlsymLibPath,
            ', RTLD_LOCAL', ');', file=output)

        for func in functions:
            print(func.type, ' (* __real_', func.name, ')(',
                ', '.join(func.parameters), '= (', func.type, '(*)(',
                ', '.join(func.parameters), ')) dlsym(dllib, (const char *) "',
                func.name, '");', file=output)

        for function in functions:

            # write function signature
            print(function.type, function.getDefinition(), end='\n{\n', sep='',
                file=output)

            # a variable to save the return-value
            returntype = function.type

            if returntype != "void":
                print('\t', returntype, ' ret;', end='\n', sep='',
                        file=output)

            # is this the desired init-function?
            if function.init:
                # write all init-templates
                for func in functions:
                    for temp in func.usedTemplates:
                        outstr = temp.output('init').strip()
                        if outstr.strip() != '':
                            print('\t', outstr, end='\n', sep='', file=output)

            # write the before-template for this function
            for temp in function.usedTemplates:
                outstr = temp.output('before').strip()
                if outstr != '':
                    print('\t', outstr, end='\n', sep='', file=output)

            # write the function call
            if returntype != "void":
                print('\tret = ', function.getPointerCall(), end=';\n',
                    sep='', file=output)
            else:
                print('\t__real_', function.getPointerCall(), end=';\n', sep='',
                        file=output)

            # is this the desired final-function?
            if function.final:
                # write all final-functions
                for func in functions:
                    for temp in func.usedTemplates:
                        outstr = temp.output('final').strip()
                        if outstr.strip() != '':
                            print('\t', outstr, end='\n', sep='', file=output)

                print("dlclose(", dllib, ");", file=output)

            # write all after-templates for this function
            for temp in function.usedTemplates:
                outstr = temp.output('after').strip()
                if outstr != '':
                    print('\t', outstr, end='\n', sep='', file=output)

            # write the return statement and close the function
            if returntype != "void":
                print('\treturn ret;\n}', end='\n\n', file=output)

            else:
                print('\n}', end='\n\n', file=output)

        # close the file
        output.close()


##
# @brief The main function.
#
def main():

  # Parse input parameter.
    opt = Option()
    options = opt.parse()

    functionParser = FunctionParser(options)
    outputWriter = Writer(options)

    functions = functionParser.parseFile()

    if options.blankHeader:
        outputWriter.headerFile(functions)

    else:
        commandParser = CommandParser(options)
        functions = commandParser.parse(functions)
        if options.style == "wrap":
            outputWriter.sourceFileWrap(functions)
        else:
            outputWriter.sourceFileDLLSym(functions)

if __name__ == '__main__':
    main()
