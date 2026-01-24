#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/httpclient.o \
	${OBJECTDIR}/httpserver.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/pacmirrors.o \
	${OBJECTDIR}/pacproxy.o \
	${OBJECTDIR}/pacserver.o \
	${OBJECTDIR}/pacworker.o


# C Compiler Flags
CFLAGS=-m64 -ffunction-sections -fdata-sections -pedantic -Wall -fsanitize=undefined -Wsuggest-override

# CC Compiler Flags
CCFLAGS=-m64 -ffunction-sections -fdata-sections -pedantic -Wall -fsanitize=undefined -Wsuggest-override
CXXFLAGS=-m64 -ffunction-sections -fdata-sections -pedantic -Wall -fsanitize=undefined -Wsuggest-override

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=--64

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pacproxy

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pacproxy: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	../lnk -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pacproxy ${OBJECTFILES} ${LDLIBSOPTIONS} -ffunction-sections -fdata-sections -fsanitize=undefined

clients.html.h: htmls/clients.html ../xrc nbproject/Makefile-${CND_CONF}.mk
	@echo Performing Custom Build Step
	../xrc -o "$@" "$<" 

index.html.h: htmls/index.html ../xrc nbproject/Makefile-${CND_CONF}.mk
	@echo Performing Custom Build Step
	../xrc -o "$@" "$<" 

packages.html.h: htmls/packages.html ../xrc nbproject/Makefile-${CND_CONF}.mk
	@echo Performing Custom Build Step
	../xrc -o "$@" "$<" 

${OBJECTDIR}/httpclient.o: httpclient.cc nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/httpclient.o httpclient.cc

${OBJECTDIR}/httpserver.o: httpserver.cc nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/httpserver.o httpserver.cc

${OBJECTDIR}/main.o: main.cc nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cc

${OBJECTDIR}/pacmirrors.o: pacmirrors.cc nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pacmirrors.o pacmirrors.cc

${OBJECTDIR}/pacproxy.o: pacproxy.cc nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pacproxy.o pacproxy.cc

${OBJECTDIR}/pacserver.o: pacserver.cc index.html.h packages.html.h clients.html.h nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pacserver.o pacserver.cc

${OBJECTDIR}/pacworker.o: pacworker.cc nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pacworker.o pacworker.cc

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} clients.html.h
	${RM} index.html.h
	${RM} packages.html.h

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
