package provide mdgg16proxy 1.0

package require snit
package require usbcontrolclient

snit::type MDGG16Proxy {

  option -module -default {}

  component _comObj

  delegate option * to _comObj

  constructor {args} {
    install _comObj using controlClient %AUTO%

    $self configurelist $args
  }

  destructor {
  }

  method SetLogicalORAB {value} {
    return [$_comObj Set [$self cget -module] "or_ab" $value]
  }

  method SetLogicalORCD {value} {
    return [$_comObj Set [$self cget -module] "or_cd" $value]
  }

  method GetLogicalORAB {} {
    return [$_comObj Get [$self cget -module] "or_ab"]
  }

  method GetLogicalORCD {} {
    return [$_comObj Get [$self cget -module] "or_cd"]
  }

}
