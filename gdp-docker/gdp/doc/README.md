% Global Data Plane --- Documentation

The documentation is now in three subdirectories; this will hopefully
clear up some confusion about what is relevant.

Those subdirectories are:

* admin --- intended for people administering GDP
  infrastructure such as log servers and other services.

* developer --- intended for developers who are writing code that
  uses the GDP as a tool.  This includes programmatic interfaces
  and network interfaces such as REST.

* internal --- documents for those working on the guts of the GDP
  itself.  Includes details of the network protocol, etc.

# A Note on Markdown

We're using a slightly extended version of Markdown.  The conversion
tool we use is Pandoc, which is extremely flexible (it converts to
nearly everything).  The extensions used are as defined by Pandoc.
Pandoc is easy to find; if you can't find it in your package manager,
try <http://pandoc.org/>.

For details on Pandoc (including the extensions to Markdown), see
<http://pandoc.org/MANUAL.html#pandocs-markdown>.

# A Note on DocBook

Some early documentation was written using DocBook.  That is mostly
phased out now in favor of Markdown.
The exception is Libep.dbk, which is still the current version.

If you still want to generate PDFs from the .DBK documents, install
the following packages:

    xsltproc
    docbook-xsl-ns
    docbook5-xml
    fop

The docbook5-xml material should be available at
<https://www.oasis-open.org/docbook/xml/5.0b5/>, or alternatively at
<http://docbook.org/xml/5.0> (better documentation here).  See also
<http://docbook.org/tdg51/en/html/appa.html> for installation advice.

_TO BE DONE: more description of printing Docbook._
