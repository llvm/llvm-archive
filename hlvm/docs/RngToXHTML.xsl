<?xml version="1.0"?><!--*- XML -*-->

<xsl:transform
  version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:rng="http://relaxng.org/ns/structure/1.0"
  xmlns:local="http://www.pantor.com/ns/local"
  xmlns:a="http://relaxng.org/ns/compatibility/annotations/1.0"
  exclude-result-prefixes="rng local a"
>
<xsl:output method="xml" index="yes" encoding="utf-8" omit-xml-declaration="no"
  doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
  doctype-public="//W3C//DTD XHTML 1.0 Strict//EN"
/>
<xsl:template match="/">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en-US" xml:lang="en-US">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    <title>Relax NG Grammar Documentation</title>
    <link rel="stylesheet" href="relaxng.css" type="text/css"/>
  </head>
  <body>
    <h1>$X Grammar</h1>
    <xsl:apply-templates/>
  </body>
</html>
</xsl:template>

<xsl:template match="/grammar">
  <div class="contents">
    <ul>
      <xsl:for-each select="start">
        <li><a href="#{@name}"><xsl:value-of select="@name"/></a></li>
      </xsl:for-each>
      <xsl:for-each select="define">
        <li><a href="#{@name}"><xsl:value-of select="@name"/></a></li>
      </xsl:for-each>
    </ul>
  </div>
  <div class="descriptions">
    <xsl:apply-templates name="start"/>
    <xsl:apply-templates select="define"/>
  </div>
</xsl:template>

<xsl:template match="start">
  <h2>Start Pattern: <xsl:value-of select="@name"/></h2>
</xsl:template>

<xsl:template name="toc" match="//define">
  <li><a href="#{@name}"><xsl:value-of select="@name"/></a></li>
</xsl:template>

<xsl:template name="define" match="//define">
</xsl:template>

</xsl:transform>
