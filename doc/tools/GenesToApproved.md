### GenesToApproved tool help
	GenesToApproved (0.1-46-gb124721)
	
	Replaces gene symbols by approved symbols using the HGNC database.
	
	Mandatory parameters:
	  -in <file>  Input TXT file with one gene symbol per line.
	
	Optional parameters:
	  -out <file> Output TXT file with approved gene symbols. If unset, writes to STDOUT.
	              Default value: ''
	  -db <file>  The HGNC flat file. If unset 'hgnc' from the 'settings.ini' file is used.
	              Default value: ''
	
	Special parameters:
	  --help      Shows this help and exits.
	  --version   Prints version and exits.
	  --tdx       Writes a Tool Defition Xml file. The file name is the application name appended with '.tdx'.
	
[back to ngs-bits](https://github.com/marc-sturm/ngs-bits)