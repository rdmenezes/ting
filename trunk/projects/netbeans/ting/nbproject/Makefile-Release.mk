#
# Gererated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=g77

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/Release/GNU-Linux-x86

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/home/postgrad/t/tw815/src/ting/trunk/src/ting.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS} dist/Release/GNU-Linux-x86/libting.so

dist/Release/GNU-Linux-x86/libting.so: ${OBJECTFILES}
	${MKDIR} -p dist/Release/GNU-Linux-x86
	${LINK.cc} -shared -o dist/Release/GNU-Linux-x86/libting.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/home/postgrad/t/tw815/src/ting/trunk/src/ting.o: ../../../src/ting.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/home/postgrad/t/tw815/src/ting/trunk/src
	$(COMPILE.cc) -O2 -fPIC  -o ${OBJECTDIR}/_ext/home/postgrad/t/tw815/src/ting/trunk/src/ting.o ../../../src/ting.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Release
	${RM} dist/Release/GNU-Linux-x86/libting.so

# Subprojects
.clean-subprojects:
