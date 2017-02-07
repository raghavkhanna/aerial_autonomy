#!/usr/bin/env python
# Generate message file based on event input
# Running the script:
#   python generate_messages.py [EVT_FILE] [EVT_FOLDER]

import sys
import os

# %%


def create_event(event_name, accumulate_event_map, out_file):
    """
    Create a class with event_name and a function to trigger
    the event. Also add the event name into a initializer list
    for creating a map of event name to the function
    Params:
      event_name         - Name of the the event class to be created
      accumulate_event_map - Variable to save initializer list into
      out_file           - Write the event class to out_file
    """
    print >>out_file, (
        "  struct {0} {{}};\n"
        "  template<class LogicStateMachine> \n"
        "  void generate_{0}(LogicStateMachine &logic_state_machine) {{\n"
        "    {0} evt;\n"
        "    logic_state_machine.process_event(evt);\n"
        "  }}\n").format(event_name)
    accumulate_event_map.append(
        '{{"{0}", generate_{0}<LogicStateMachine>}}'.format(event_name))


def create_event_manager(event_manager_name, accumulate_event_map, out_file):
    """
    Create event manager class. This class provides functions to trigger
    an event by name, and print all the events managed by the class. The
    event manager is enclosed by event_file_name namespace to distinguish
    between different event files containing similar events and event managers
    """
    print >>out_file, (
        "  template<class LogicStateMachine>\n"
        "  class {0} {{\n"
        "    typedef std::function<void(LogicStateMachine&)> EventFunction;\n"
        "    std::map<std::string, EventFunction> evt_map = {{\n\t{1},\n"
        "    }};\n"
        "    public:\n"
        "    void triggerEvent(std::string evt_name,"
        " LogicStateMachine& logic_state_machine) {{\n"
        "      evt_map[evt_name](logic_state_machine);\n"
        "      return;\n"
        "    }}\n"
        "    void printEventList() {{\n"
        "      std::cout<<\"Events: \"<<std::endl;\n"
        "      for(auto &s : evt_map) {{\n"
        "        std::cout<<s.first<<std::endl;\n"
        "      }}\n"
        "    }}\n"
        "  }};").format(event_manager_name, ',\n\t'.join(accumulate_event_map),)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Cannot create a header file without events and event folder"
        print sys.argv
        sys.exit(-1)
    # Open evt file
    f = open(sys.argv[1], 'r')
    evt_file_base = os.path.basename(sys.argv[1])
    evt_file_wext = os.path.splitext(evt_file_base)[0] + '.h'
    # Create header file
    out_file = open(os.path.join(sys.argv[2], evt_file_wext), 'w')
    # Print header
    print >> out_file, (
        "/** Auto generated header file from event file\n"
        "  Do not edit this file manually **/\n"
        "#include<functional>\n"
        "#include<iostream>\n"
        "#include <map>\n"
    )
    # Enclose the events and event manager into namespace
    print >> out_file, "namespace %s {" % (evt_file_base,)
    accumulate_event_map = []
    event_names_list = f.read().splitlines()
    event_manager_name = event_names_list[0][:-1]
    for event_name in event_names_list[1:]:
        event_name = event_name.strip()
        create_event(event_name, accumulate_event_map, out_file)
    print >>out_file, "\n//Event manager class"
    create_event_manager(event_manager_name, accumulate_event_map, out_file)
    print >>out_file, "}"
    out_file.close()
