# Flow_Digital
This flow was developed to make the synthesis and placement process for digital and mixed-signal integrated circuits (ICs) an accessible and productive procedure. The flow alleviates the in-depth technical knowledge required to perform a physical implementation, which is specific for every set of tools, by creating an eXtensible Markup Language (XML) interface. The user can describe their design using XML structures, feed them to the scripts and receive a full physical implementation that follows the specifications.

## Dependencies
A list of the required EDA tools and Linux packages to run the flow, and their tested for compatibility version, is included below.
* EDA tools
  * Cadence Genus (>= 19)
  * Cadence Innovus (= 20)
  * Cadence Virtuoso (= IC618)
    * strmout
    * idhl
    * strmin
    * abstract
    * si
  * Cadence Spectre (= 20)
  * Cadence XCELIUM (= 2009)
  * Synopsys VCS (= 2018.09-SP2-10)
  * Mentor Graphics Calibre (= 2021.3_15.9)
* Packages
  * tclsh (>= 8.5)
  * git
  * xmllint
  * GNU realpath
  * readlink
  * sed
  * awk
  * grep
  * make
* Core Utilities:
  * mkdir
  * rm
  * tail
  * cat
  * cp
  * mv
  * chmod
  * cat
  * touch

## Configuration Files
The configuration files comprise all the inputs of the flow (such as location of files, performance specifications and physical measurements). These parameters are logically split in segments to make it easier to comprehend, navigate and modify them based on the desired result.<br />
**Technology**: Contains information about the PDK. Its name, information about the node, names, location, views of the standard cell libraries (SCLibs), units for the SCLib characterization files, mapping between the different PDK formats, lists of purpose specific cells (e.g., filler, clock) and details of the voltage domain.<br />
**Design**: Contains information about the high-level digital design. The name of the top module, the list of synthesizable and testbenches sources and constraints (delay, transition time, load etc.) of the I/O ports.<br />
**Mapping**: Contains information about the implementation of the design in the specific technology. Its operational conditions (threshold voltage, temperature etc.), custom standard cell libraries (additional to the ones provided with the PDK), synthesis parameters (e.g., use of specific cells) and the physical configuration (positioning of pins, sizing of rings and grid).<br />
**Project**: Contains information about directory structure used form the flow (useful for modifying the paths for parallelization).<br />

## Execution
The simplification of the execution process was also a significant goal during the development, so a Makefile to handle the data-routing and, also, act as a summary of the operations that the flow can execute. Most of the commands take as arguments a subset of the configuration segments; Technology (TECH), Design (DSGN), Mapping (MAP), Project (PROJ).

**sim**: Runs a behavioral simulation of the design using Synopsys VCS. The command (CMD) argument takes value from the list (RTL, SYN, PNR) and executes the source design files, the synthesized or the physical netlist respectively. The simulated waveforms are stored in .vcd and .vpd formats in the simulation output directory based on the dumpvars command specified in the testbench.<br />
**wave**: Opens an GUI instance of the Synopsys DVE to scope the simulated waveforms.<br />
**syn**: Executes the synthesis scripts on Cadence Genus  using the specified Register Transfer Level (RTL) sources. The synthesized netlist is exported in the synthesis output directory segment and follows the format Top_module.syn.v alongside several reports and log.<br />
**pnr**: Executes the place-and-routing scripts on Cadence Innovus3 using the output of the synthesis operation. The physical layout is exported in the placement output directory in GDS II format and follow the format Top_module.gds alongside several reports and log.<br />
**rundir**: Creates the execution directory used by Cadence Virtuoso, based on the template specified in the technology configuration segment, which is the basis of several operations.<br />
**layout**: Creates a library compatible with Cadence Virtuoso in the library output directory and converts the physical layout exported by the place-and-route operation and stores it.<br />
**schematic**: Creates the schematic representation of the physical layout and attaches it in the library formed by the layout operation.<br />
**abstract**: Exports the physical characteristics of the physical design in a .lef format.<br />
**drc**: Runs a Design Rule Check (DRC) using Mentor Graphics Calibre on the physical design based on the template specified in the technology configuration segment.<br />
**lvs**: Runs a Design Rule Check (DRC) using Mentor Graphics Calibre on the physical design based on the template specified in the technology configuration segment.<br />
**summary**: Extracts the results of the reports from several operations and creates a succinct summary of them (under development).<br />
**virtuoso**: Run Cadence Virtuoso in the proper execution directory.<br />
**tidy**: Deletes open files created by the user.<br />
**clean**: Deletes temporary files created by the scripts.<br />
**nuke**: Deletes any non-checked-in file.

The scripts can run independently, but it is suggested to be executed through the Makefile to maximize chances of compatibility with future versions. Also, the proposed order of execution is the one used in the presentation of them and the attached Makefile.
