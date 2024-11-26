#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-Dom32Touch.mk)" "nbproject/Makefile-local-Dom32Touch.mk"
include nbproject/Makefile-local-Dom32Touch.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=Dom32Touch
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/Programa.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/Programa.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=main.c sd/fsio.c sd/sd_spi.c log.c flash/plib_nvm.c pgm.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/main.o ${OBJECTDIR}/sd/fsio.o ${OBJECTDIR}/sd/sd_spi.o ${OBJECTDIR}/log.o ${OBJECTDIR}/flash/plib_nvm.o ${OBJECTDIR}/pgm.o
POSSIBLE_DEPFILES=${OBJECTDIR}/main.o.d ${OBJECTDIR}/sd/fsio.o.d ${OBJECTDIR}/sd/sd_spi.o.d ${OBJECTDIR}/log.o.d ${OBJECTDIR}/flash/plib_nvm.o.d ${OBJECTDIR}/pgm.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/main.o ${OBJECTDIR}/sd/fsio.o ${OBJECTDIR}/sd/sd_spi.o ${OBJECTDIR}/log.o ${OBJECTDIR}/flash/plib_nvm.o ${OBJECTDIR}/pgm.o

# Source Files
SOURCEFILES=main.c sd/fsio.c sd/sd_spi.c log.c flash/plib_nvm.c pgm.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-Dom32Touch.mk ${DISTDIR}/Programa.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MM0256GPM028
MP_LINKER_FILE_OPTION=,--script="boot_p32MM0256GPM028.ld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/main.o: main.c  .generated_files/flags/Dom32Touch/4d323b3a659dead48d32281900c5291915da9791 .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/main.o.d" -o ${OBJECTDIR}/main.o main.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/sd/fsio.o: sd/fsio.c  .generated_files/flags/Dom32Touch/aba28d6d0ac957f9e11669a536a9c80bcebe924c .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/sd" 
	@${RM} ${OBJECTDIR}/sd/fsio.o.d 
	@${RM} ${OBJECTDIR}/sd/fsio.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/sd/fsio.o.d" -o ${OBJECTDIR}/sd/fsio.o sd/fsio.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/sd/sd_spi.o: sd/sd_spi.c  .generated_files/flags/Dom32Touch/393c4a9a7bd16b072571e2e59c883ea11746816b .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/sd" 
	@${RM} ${OBJECTDIR}/sd/sd_spi.o.d 
	@${RM} ${OBJECTDIR}/sd/sd_spi.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/sd/sd_spi.o.d" -o ${OBJECTDIR}/sd/sd_spi.o sd/sd_spi.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/log.o: log.c  .generated_files/flags/Dom32Touch/9aa5f71b2fe0673d47524d9bac3215789d1daa80 .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/log.o.d 
	@${RM} ${OBJECTDIR}/log.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/log.o.d" -o ${OBJECTDIR}/log.o log.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/flash/plib_nvm.o: flash/plib_nvm.c  .generated_files/flags/Dom32Touch/147489f7865ab4d56c57c0f88ec86948f4eb919 .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/flash" 
	@${RM} ${OBJECTDIR}/flash/plib_nvm.o.d 
	@${RM} ${OBJECTDIR}/flash/plib_nvm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/flash/plib_nvm.o.d" -o ${OBJECTDIR}/flash/plib_nvm.o flash/plib_nvm.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/pgm.o: pgm.c  .generated_files/flags/Dom32Touch/2abeb5649f40aec855c290288c5461d7597bf0bc .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pgm.o.d 
	@${RM} ${OBJECTDIR}/pgm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/pgm.o.d" -o ${OBJECTDIR}/pgm.o pgm.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
else
${OBJECTDIR}/main.o: main.c  .generated_files/flags/Dom32Touch/ae868bab82887b9da843c4fbd4d42a4c0774a2b8 .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/main.o.d" -o ${OBJECTDIR}/main.o main.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/sd/fsio.o: sd/fsio.c  .generated_files/flags/Dom32Touch/d608c5b1b56654b8fd9fe9d508cdfaaae6ab06f9 .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/sd" 
	@${RM} ${OBJECTDIR}/sd/fsio.o.d 
	@${RM} ${OBJECTDIR}/sd/fsio.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/sd/fsio.o.d" -o ${OBJECTDIR}/sd/fsio.o sd/fsio.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/sd/sd_spi.o: sd/sd_spi.c  .generated_files/flags/Dom32Touch/8c275fb5b5ec8458a02be6647e2a79d5e0db2e48 .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/sd" 
	@${RM} ${OBJECTDIR}/sd/sd_spi.o.d 
	@${RM} ${OBJECTDIR}/sd/sd_spi.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/sd/sd_spi.o.d" -o ${OBJECTDIR}/sd/sd_spi.o sd/sd_spi.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/log.o: log.c  .generated_files/flags/Dom32Touch/210f6741c22213da40b9d3d72c2505c53967a789 .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/log.o.d 
	@${RM} ${OBJECTDIR}/log.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/log.o.d" -o ${OBJECTDIR}/log.o log.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/flash/plib_nvm.o: flash/plib_nvm.c  .generated_files/flags/Dom32Touch/378941536333df3281bc1f98741ed4f7ef9fb64 .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/flash" 
	@${RM} ${OBJECTDIR}/flash/plib_nvm.o.d 
	@${RM} ${OBJECTDIR}/flash/plib_nvm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/flash/plib_nvm.o.d" -o ${OBJECTDIR}/flash/plib_nvm.o flash/plib_nvm.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/pgm.o: pgm.c  .generated_files/flags/Dom32Touch/4ac6ab67cf37022d829c8a4659cb16849dfbafd .generated_files/flags/Dom32Touch/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pgm.o.d 
	@${RM} ${OBJECTDIR}/pgm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -Os -fno-common -Wall -MP -MMD -MF "${OBJECTDIR}/pgm.o.d" -o ${OBJECTDIR}/pgm.o pgm.c    -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -DDOM32TOUCH -mdfp="${DFP_DIR}"  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/Programa.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    boot_p32MM0256GPM028.ld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g -mdebugger -D__MPLAB_DEBUGGER_PK3=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -O2 -o ${DISTDIR}/Programa.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)   -mreserve=data@0x0:0x1FC -mreserve=boot@0x1FC00490:0x1FC016FF -mreserve=boot@0x1FC00490:0x1FC00BEF  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=__MPLAB_DEBUGGER_PK3=1,--no-check-sections,--gc-sections,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}"
	
else
${DISTDIR}/Programa.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   boot_p32MM0256GPM028.ld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -O2 -o ${DISTDIR}/Programa.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_Dom32Touch=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--no-check-sections,--gc-sections,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}"
	${MP_CC_DIR}\\xc32-bin2hex ${DISTDIR}/Programa.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
