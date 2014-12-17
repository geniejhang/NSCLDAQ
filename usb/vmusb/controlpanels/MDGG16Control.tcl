

package require mdgg16gui
package require mdgg16proxy



MDGG16Proxy ::proxy -server localhost -port 27000 -module mymdgg16

MDGG16View .view 
MDGG16Presenter ::pres 
::pres configure -view .view
::pres configure -handle ::proxy

grid .view -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1

