#
#  SEE Spectrum configuration file.
#

# Spectra for all  parameters at full resolution:

   # The ppac spectra:

foreach item {u d l r a} {
    spectrum see.ppac.$item 1 see.ppac.$item 12
}
spectrum see.ppac.x 1 see.ppac.x {{-6.0 6.0 500}}
spectrum see.ppac.y 1 see.ppac.y {{-6.0 6.0 500}}
spectrum see.ppac.y_vs_x 2 {see.ppac.x see.ppac.y} \
    {{-6.0 6.0 500} {-6.0 6.0 500}}

   # The scintillator raw spectra:

foreach item {u d l r} {
    spectrum see.sci.$item 1 see.sci.$item 12
}

  # Calculated spectra (not really from parameters).

#  Profile spectra these are really just scaled x/y positions.
#  The scaling, however is done in calculating the parameters.
#
#   These are bogus right now...
#
spectrum see.ppac.x_profile 1 see.ppac.x_profile 12
spectrum see.ppac.y_profile 1 see.ppac.y_profile 12


#   The spectrum below isn't really from parameters, it
#   is just 4 channels that contain incremental (?) scaler
#   counts for the see scalers.

spectrum see.sci.counts 1 see.sci.counts 2  ;# 4 channels.

