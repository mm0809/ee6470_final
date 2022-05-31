#################################################
# Stratus IDE project file generated;
#################################################
QT       += core

QT       -= gui

CONFIG   += console
CONFIG   -= app_bundle

STRATUSHOME = $$(STRATUS_HOME)
SYSTEMCHOME = $$(SYSTEMC)

TEMPLATE = app
INCLUDEPATH += $${SYSTEMCHOME}/include
INCLUDEPATH += $${SYSTEMCHOME}/include/tlm
INCLUDEPATH += $${STRATUSHOME}/share/stratus/include
INCLUDEPATH += ./bdw_work/wrappers
INCLUDEPATH += ./

SOURCES += \ 
		../main.cpp \ 
		../Testbench.cpp \ 
		../System.cpp \ 
		../MergeW1.cpp \ 
		../MergeW2.cpp \ 
		../MergeW4.cpp \ 
		../MergeW8.cpp \ 

HEADERS += \ 
		../Testbench.h \ 
		../System.h \ 
		../MergeW1.h \ 
		../MergeW2.h \ 
		../MergeW4.h \ 
		../MergeW8.h \ 

OTHER_FILES += \ 
		Makefile \ 
		/home/u108/u108061111/EE6470/midterm/midterm_project_multistage/stratus/project.tcl \ 

