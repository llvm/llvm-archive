# APPLE LOCAL file string workaround 4943900
if { [istarget "*-*-darwin9*"] } {
  set additional_flags "-framework Foundation -fconstant-cfstrings"
}
return 0
# APPLE LOCAL file string workaround 4943900
if { [istarget "*-*-darwin9*"] } {
  set additional_flags "-framework Foundation -fconstant-cfstrings"
}
return 0
