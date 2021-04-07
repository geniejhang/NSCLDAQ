#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file mg_pgmstatusclientui.tcl
# @brief Provides megawidgets to display program status information.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide pgmstatusclientui 1.0
lappend auto_path $::env(DAQTCLLIBS)
package require programstatusclient
package require Tk
package require snit

##
# @class ContainerStatusList
#    Provides a list of containers and their activations.
#    This is presented as a ttk::treeview with the following columns:
#    *  name - name of the container.
#    *  image - path to containe image
#    *  binding - filesystem binding (see below).
#    *  host    - activation host (see below).
#
#   Each container is represented as a top level item in the tree with
#   two sub elements called "bindings" and "activations"
#   The bindings subelement has subelements for each binding and those
#   subelements populate the binding column.
#   The activations subelement has subelelements for each activation of the
#   container and those populate the host column.
#
# Naturally this megawidget provides a vertical scroll bar.
#
# OPTIONS:
#   -containers   - list of container definition dicts gotten from e.g.
#                   a call to ProgramClient::status on an instance.
# METHOD:
#   addActivation - Adds an activation to a container.
#   removeActivation - Removes an activation from a container.
#
#  The relative paucity of methods is because the things that are most likely
#  to vary dynamically are the activations of a container and we want to
#  be able to update those without the user seeing the entire tree close as it
#  would if we re-generated the tree from scratch.
#
snit::widgetadaptor ContainerStatusList {
    option -containers -default [list] -configuremethod _cfgContainers
    component tree
    constructor {args} {
        installhull using ttk::frame
        
        set colnames [list  image binding host]
        set coltitles [list Image Binding Host]
        
        install tree using ttk::treeview $win.tree                 \
            -columns $colnames                                     \
            -displaycolumns $colnames -show [list tree headings]    \
            -selectmode none -yscrollcommand [list $win.ysb set]
        foreach col $colnames title $coltitles {
            $tree heading $col -text $title
        }
        $tree heading #0 -text Container;   # Label the tree header.
        
        ttk::scrollbar $win.ysb -orient vertical -command [list $tree yview]
        
        grid $tree $win.ysb -sticky nsew
        grid columnconfigure $win 0 -weight 1;   # Allow the treeview to grow
        grid columnconfigure $win 1 -weight 0;   # But not the scrollbar.
        
        $self configurelist $args
        
    }
    #------------------------------------------------------------------------
    # Private utility methods.
    #
    
    ##
    # _addContainer
    #    Add a container to the end of the set of things in the tree.#
    #      Top level has name, image, bindings where name is the tree.
    #      second level is bindings and activations in the tree level
    #      etc.
    #
    # @param item - dictionary item that describes the container to add.
    #
    method _addContainer {item} {
        set name [dict get $item name]
        set image [dict get $item image]
        
        set toplevel [$tree insert {} end -text $name -values $image -tags container]
        
        #  The bindings:
        
        set bindings [$tree insert $toplevel end -text bindings -tags bindcontainer]
        foreach binding [dict get $item bindings] {
            $tree insert $bindings end -values [list "" $binding] -tags binding
        }
        # Any activiations:
        
        set activations [$tree insert $toplevel end -text activations -tags activationcontainer]
        foreach host [dict get $item activations] {
            $tree insert $activations end -values [list "" "" $host] -tags activation
        }  
    }
    ##
    # _findContainer
    #    Find the entry that corresponds to the named container.
    #
    #  @param name - name of the container.
    #  @return entry - entry id of the container top level.
    #  @note it's an error for there not to be a match.
    #
    method _findContainer {name} {
        foreach item [$tree children {}] {
            if {[$tree item $item -text] eq $name} {
                return $item
            }
        }
        error "No such container: $name"
    }
    ##
    # _findActiviations
    #    Find the subelement of a container that holds the activations.
    #
    # @param c  - container item (e.g. from _findContainer).
    # @return entry - activations entry.
    # @note it's an error for there not to be an activation entry.
    #
    method _findActivations {c} {
        foreach sub [$tree children $c] {
            if {[$tree item $sub -text] eq "activations"} {
                return $sub
            }
        }
        error "The container [$tree item $c -text] does not have an activations subtree"
    }
    ##
    # _addOptionActivation
    #
    #  Searches the option list for the named container and
    #  lappends an activation record to it.
    #
    # @param name   - container name.
    # @param host   - Host the container has been activated on.
    # @note it is an error 'name' does not exist.
    #
    method _addOptionActivation {name host} {
        set containers $options(-containers)
        set index 0
        foreach container $containers {
            if {[dict get $container name] eq $name} {
                dict lappend container activations $host
                lset containers $index $container
                set options(-containers) $containers
                return 
            }
            incr index
        }
        error "The -containers option has no match to $name"
    }
    
    ##
    # _addTreeActivation
    #
    #    Searchs the tree for matching named container and adds
    #    an activation record to it.
    #
    # @param name - name of the container.
    # @param host - Host of new activation.
    # @note it is an error if does not exist.
    #
    method _addTreeActivation {name host} {
        set child [$self _findContainer $name]
        set subtree [$self _findActivations $child]
        $tree insert $subtree end  -values [list "" "" $host] -tags activation    
        return
    }
    
    ##
    # _removeOptionActivation
    #   Removes an activation entry from the options.
    #
    # @param name   - name of the container.
    # @param host   - host in which it's being deactivated.
    # @note name must be an existing container active on host.
    #
    method _removeOptionActivation {name host} {
        set containers $options(-containers)
        set index 0
        foreach container $containers {
            if {[dict get $container name] eq $name} {
                set activations [dict get $container activations]
                
                set which [lsearch -exact $activations $host]
                if {$which == -1} {
                    error '$name is no active on $host
                }
                
                set activations [lreplace $activations $which $which]
                dict set container activations $activations
                lset containers $index $container
                
                set options(-containers) $containers
                
                return
            }
            incr index
        }
        error "There is no container named $name in -containers"
    }
    ##
    # _removeTreeActivation
    #   Removes an activation entry from a container in the tree.
    #
    # @param name - container name.
    # @param host - host in which it's being deactivated.
    #
    method _removeTreeActivation {name host} {
        set child [$self _findContainer $name]
        set sub   [$self _findActivations $child]
    
        foreach a [$tree children $sub] {
            set h [lindex [$tree item $a -values] 2]
            if {$h eq $host} {
                $tree delete $a
                return
            }
        }
        #  No activation:
        
        error "Tree has no activation on $host of $name"

}
    
    
    #------------------------------------------------------------------------
    #  Configuration handling
    
    ##
    # _cfgContainers
    #    Called when -containers is configured.
    #    - Clear the tree
    #    - Repopulate it using the new value.
    # @param optname - option name.
    # @param optval  - new option value.
    # @note  options(optname) is set so we don't need a cget operation.
    #
    method _cfgContainers {optname optval} {
        foreach child [$tree children {}] {
            $tree delete $child;    # Deletes the subierarchy.
        }
        
        foreach item $optval {
            $self _addContainer $item
        }
        # If there are no errors, we can update:
        
        set options($optname) $optval
    }
    #----------------------------------------------------------------------
    #  Public methods
    
    ##
    # addActivation
    #    adds an activation to an existing container
    #
    # @param name   - Name of the container
    # @param host   - Host to add to the activations.
    # @note nothing is done to detect/reject duplicates.
    # @note it is an error to specify a nonexisting container name.
    # @note The corresponding item in the -container option list is
    #       modified. while the tree is modified in place so that
    #       nothing prematurely closes.
    #
    method addActivation {name host} {
        $self _addOptionActivation $name $host
        $self _addTreeActivation $name $host
    }
    ##
    # removeActivation
    #
    #     Removes activation from an existing container.
    #
    # @param name   - Name of the container.
    # @param host   - Host on which that container is no longer active.
    # @note nonexistent containers and nonexistent active hosts in the
    #       container activation list are errors.
    # @note as with addActivation, the -containers option value is updated
    #       to be accurate.
    #
    method removeActivation {name host} {
        $self _removeOptionActivation $name $host
        $self _removeTreeActivation $name $host
    }
     
}
    


