package require ReadoutStatModel
package require ReadoutStatView
package require ReadoutStatController
package require statusMessage


set db [statusdb create $argv readonly]

set model [ReadoutStatModel %AUTO% -dbcommand $db]
set view  [ReadoutStatView .v]
set controller [ReadoutStatController %AUTO% -model $model -view $view]

pack $view -fill both -expand 1
