#
#  Script to update a license block
#
#	Replaces everything between start/stop headers with a new license
#

BEGIN {
    saveRS = RS
    RS = "^$"
    licenseFile = "LICENSE"
    getline licenseText < licenseFile
    sub(/\n$/, "", licenseText)
    close(licenseFile)
    RS = saveRS
}
/----- BEGIN LICENSE BLOCK -----/ {
    if ($1 == "-----") $1 = ""
    prefix = "\n" $1 "\t"
    gsub(/\n/, prefix, licenseText)
    gsub(/[\t ]*\n/, "\n", licenseText)		# strip trailing white space
    print
    print $1 "\t" licenseText
    skipText = 1
}
/----- END LICENSE BLOCK -----/ {
    skipText = 0
}
skipText == 0 {
    print
}
