########################################################################
# Makefile for 3D Visualizer, a generic visualization program for 3D
# multivariate gridded data.
# Copyright (c) 1999-2008 Oliver Kreylos
#
# This file is part of the WhyTools Build Environment.
# 
# The WhyTools Build Environment is free software; you can redistribute
# it and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
# 
# The WhyTools Build Environment is distributed in the hope that it will
# be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the WhyTools Build Environment; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA
########################################################################

# Root directory of the Vrui software installation. This must match the
# same setting in Vrui's makefile. By default the directories match; if
# the installation directory was adjusted during Vrui's installation, it
# must be adjusted here as well.
VRUIDIR = /home/jrevote/Documents/Projects/3D-Alive/Development/Builds/Vrui-1.0-050

# Base installation directory for 3D Visualizer and its module
# plug-ins. The module plug-ins cannot be moved from this location
# after installation, or 3D Visualizer will not find them. If this is
# set to the default of $(PWD), 3D Visualizer does not have to be
# installed to be run. 3D Visualizer's executable, plug-ins, and
# resources will be installed in the bin, lib (or lib64), and share
# directories underneath the given base directory, respectively.
INSTALLDIR = /home/jrevote/Documents/Projects/3D-Alive/Development/Builds/3DVisualizer-1.0

# Set up additional flags for the C++ compiler:
CFLAGS = 

# List of default visualization modules:
MODULE_NAMES = SphericalASCIIFile \
               StructuredGridASCII \
               StructuredGridVTK \
               CitcomCUCartesianRawFile \
               CitcomCUSphericalRawFile \
               CitcomSRegionalASCIIFile \
               CitcomSGlobalASCIIFile \
               CitcomtFile \
               MultiCitcomtFile \
               CitcomtVectorFile \
               ImageStack

# List of other available modules:
UNSUPPORTED_MODULE_NAMES = AnalyzeFile \
                           ByteVolFile \
                           GaleFEMVectorFile \
                           ConvectionFile \
                           ConvectionFileCartesian \
                           CSConvectionFile \
                           DicomImageStack \
                           EarthTomographyGrid \
                           ReifSeismicTomography \
                           SeismicTomographyModel \
                           FloatGridFile \
                           FloatVolFile \
                           VecVolFile \
                           Kollmann0p9File \
                           MagaliSubductionFile \
                           MargareteSubductionFile \
                           VanKekenFile \
                           UnstructuredPlot3DFile

# Set up destination directories for compilation products:
OBJDIRBASE = o
BINDIRBASE = bin
MODULEDIRBASE = 3DVisualizer-1.0

# Set resource directory:
RESOURCEDIR = share/3DVisualizer-1.0

# Create debug or fully optimized versions of the software:
ifdef DEBUG
  # Include the debug version of the Vrui application makefile fragment:
  include $(VRUIDIR)/etc/Vrui.debug.makeinclude
  # Enable debugging and disable optimization:
  CFLAGS += -g3 -O0
  # Set destination directories for created objects:
  OBJDIR = $(OBJDIRBASE)/debug
  BINDIR = $(BINDIRBASE)/debug
  MODULEDIR = $(MODULEDIRBASE)/debug
else
  # Include the release version of the Vrui application makefile fragment:
  include $(VRUIDIR)/etc/Vrui.makeinclude
  # Disable debugging and enable optimization:
  CFLAGS += -g0 -O3 -DNDEBUG
  # Set destination directories for created objects:
  OBJDIR = $(OBJDIRBASE)
  BINDIR = $(BINDIRBASE)
  MODULEDIR = $(MODULEDIRBASE)
endif

# Add base directory to include path:
CFLAGS += -I.

# Pattern rule to compile C++ sources:
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)/$(*D)
	@echo Compiling $<...
	@/usr/local/packages/gcc-4.0.1/bin/g++ -c -o $@ $(VRUI_CFLAGS) $(CFLAGS) $<

# System-default destination directory for dynamic libraries:
ifeq (`uname -m`,x86_64)
  LIBDIR = lib64
else
  LIBDIR = lib
endif

# Function to generate full plug-in names:
PLUGINNAME = $(LIBDIR)/$(MODULEDIR)/lib$(1).$(VRUI_PLUGINFILEEXT)

# Pattern rule to link visualization module plug-ins:
$(call PLUGINNAME,%): CFLAGS += $(VRUI_PLUGINCFLAGS)
$(call PLUGINNAME,%): $(OBJDIR)/Concrete/%.o
	@mkdir -p $(LIBDIR)/$(MODULEDIR)
	@echo Linking $@...
	@/usr/local/packages/gcc-4.0.1/bin/g++ $(VRUI_PLUGINLINKFLAGS) -o $@ $^

# Rule to build all 3D Visualizer components:
MODULES = $(MODULE_NAMES:%=$(call PLUGINNAME,%))
ALL = $(BINDIR)/3DVisualizer \
      $(MODULES)
.PHONY: all
all: $(ALL)

# Rule to remove build results:
clean:
	-rm -f $(OBJDIR)/*.o
	-rm -f $(ALL)

# Rule to clean the source directory for packaging:
distclean:
	-rm -rf $(OBJDIRBASE)
	-rm -rf $(BINDIRBASE)
	-rm -rf lib lib64

# List of required source files:
ABSTRACT_SOURCES = Abstract/Module.cpp \
                   Abstract/ScalarExtractor.cpp \
                   Abstract/VectorExtractor.cpp \
                   Abstract/DataSet.cpp \
                   Abstract/DataSetRenderer.cpp \
                   Abstract/Element.cpp \
                   Abstract/Algorithm.cpp

TEMPLATIZED_SOURCES = Templatized/Simplex.cpp \
                      Templatized/Tesseract.cpp \
                      Templatized/SliceCaseTableSimplex.cpp \
                      Templatized/SliceCaseTableTesseract.cpp \
                      Templatized/IsosurfaceCaseTableSimplex.cpp \
                      Templatized/IsosurfaceCaseTableTesseract.cpp

WRAPPERS_SOURCES = Wrappers/RenderArrow.cpp \
                   Wrappers/SlicedScalarVectorDataValue.cpp

VISUALIZER_SOURCES = $(ABSTRACT_SOURCES) \
                     $(TEMPLATIZED_SOURCES) \
                     $(WRAPPERS_SOURCES) \
                     VolumeRenderer.cpp \
                     PaletteRenderer.cpp \
                     BaseLocator.cpp \
                     CuttingPlaneLocator.cpp \
                     EvaluationLocator.cpp \
                     ScalarEvaluationLocator.cpp \
                     VectorEvaluationLocator.cpp \
                     DataLocator.cpp \
                     ColorBar.cpp \
                     ColorMap.cpp \
                     PaletteEditor.cpp \
                     Visualizer.cpp

# Per-source compiler flags:
$(OBJDIR)/Visualizer.o: CFLAGS += -DVISUALIZER_MODULENAMETEMPLATE='"$(INSTALLDIR)/$(call PLUGINNAME,%s)"'

$(BINDIR)/3DVisualizer: $(VISUALIZER_SOURCES:%.cpp=$(OBJDIR)/%.o)
	@mkdir -p $(BINDIR)
	@echo Linking $@...
	@/usr/local/packages/gcc-4.0.1/bin/g++ -o $@ $^ $(VRUI_LINKFLAGS) $(VRUI_PLUGINHOSTLINKFLAGS)
.PHONY: 3DVisualizer
3DVisualizer: $(BINDIR)/3DVisualizer

# Dependencies and special flags for visualization modules:
$(call PLUGINNAME,SphericalASCIIFile): CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(call PLUGINNAME,SphericalASCIIFile): $(OBJDIR)/Concrete/SphericalASCIIFile.o \
                                       $(OBJDIR)/Concrete/PointSet.o \
                                       $(OBJDIR)/Concrete/EarthRenderer.o

$(call PLUGINNAME,CitcomCUSphericalRawFile): CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(call PLUGINNAME,CitcomCUSphericalRawFile): $(OBJDIR)/Concrete/CitcomCUSphericalRawFile.o \
                                             $(OBJDIR)/Concrete/PointSet.o \
                                             $(OBJDIR)/Concrete/EarthRenderer.o

$(call PLUGINNAME,CitcomSRegionalASCIIFile): CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(call PLUGINNAME,CitcomSRegionalASCIIFile): $(OBJDIR)/Concrete/CitcomSRegionalASCIIFile.o \
                                             $(OBJDIR)/Concrete/CitcomSCfgFileParser.o \
                                             $(OBJDIR)/Concrete/PointSet.o \
                                             $(OBJDIR)/Concrete/EarthRenderer.o

$(call PLUGINNAME,CitcomSGlobalASCIIFile): CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(call PLUGINNAME,CitcomSGlobalASCIIFile): $(OBJDIR)/Concrete/CitcomSGlobalASCIIFile.o \
                                           $(OBJDIR)/Concrete/CitcomSCfgFileParser.o \
                                           $(OBJDIR)/Concrete/PointSet.o \
                                           $(OBJDIR)/Concrete/EarthRenderer.o

$(call PLUGINNAME,CitcomtFile): CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(call PLUGINNAME,CitcomtFile): $(OBJDIR)/Concrete/CitcomtFile.o \
                                $(OBJDIR)/Concrete/PointSet.o \
                                $(OBJDIR)/Concrete/EarthRenderer.o

$(call PLUGINNAME,MultiCitcomtFile): CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(call PLUGINNAME,MultiCitcomtFile): $(OBJDIR)/Concrete/MultiCitcomtFile.o \
                                     $(OBJDIR)/Concrete/PointSet.o \
                                     $(OBJDIR)/Concrete/EarthRenderer.o

$(call PLUGINNAME,CitcomtVectorFile): CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(call PLUGINNAME,CitcomtVectorFile): $(OBJDIR)/Concrete/CitcomtVectorFile.o \
                                      $(OBJDIR)/Concrete/PointSet.o \
                                      $(OBJDIR)/Concrete/EarthRenderer.o

$(call PLUGINNAME,DicomImageStack): $(OBJDIR)/Concrete/DicomImageStack.o \
                                    $(OBJDIR)/Concrete/DicomImageFile.o

$(call PLUGINNAME,EarthTomographyGrid): CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(call PLUGINNAME,EarthTomographyGrid): $(OBJDIR)/Concrete/EarthTomographyGrid.o \
                                        $(OBJDIR)/Concrete/PointSet.o \
                                        $(OBJDIR)/Concrete/EarthRenderer.o

$(call PLUGINNAME,ReifSeismicTomography): CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(call PLUGINNAME,ReifSeismicTomography): $(OBJDIR)/Concrete/ReifSeismicTomography.o \
                                          $(OBJDIR)/Concrete/PointSet.o \
                                          $(OBJDIR)/Concrete/EarthRenderer.o

$(call PLUGINNAME,SeismicTomographyModel): CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(call PLUGINNAME,SeismicTomographyModel): $(OBJDIR)/Concrete/SeismicTomographyModel.o \
                                           $(OBJDIR)/Concrete/PointSet.o \
                                           $(OBJDIR)/Concrete/EarthRenderer.o

$(call PLUGINNAME,VanKekenFile): CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(call PLUGINNAME,VanKekenFile): $(OBJDIR)/Concrete/VanKekenFile.o \
                                 $(OBJDIR)/Concrete/PointSet.o \
                                 $(OBJDIR)/Concrete/EarthRenderer.o

# Keep module object files around after building:
.SECONDARY: $(MODULE_NAMES:%=$(OBJDIR)/Concrete/%.o)

install: $(ALL)
	@echo Installing 3D Visualizer in $(INSTALLDIR)...
	@install -d $(INSTALLDIR)
	@install -d $(INSTALLDIR)/bin
	@install $(BINDIR)/3DVisualizer $(INSTALLDIR)/bin
	@install -d $(INSTALLDIR)/$(LIBDIR)/$(MODULEDIR)
	@install $(MODULES) $(INSTALLDIR)/$(LIBDIR)/$(MODULEDIR)
	@install -d $(INSTALLDIR)/$(RESOURCEDIR)
	@install $(RESOURCEDIR)/EarthTopography.png $(RESOURCEDIR)/EarthTopography.ppm $(INSTALLDIR)/$(RESOURCEDIR)
