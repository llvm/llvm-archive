<?xml version="1.0"?><!--*- XML -*-->

<xsl:transform
  version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:rng="http://relaxng.org/ns/structure/1.0"
  xmlns:local="http://www.pantor.com/ns/local"
  xmlns:a="http://relaxng.org/ns/compatibility/annotations/1.0"
  exclude-result-prefixes="rng local a"
>
<xsl:template match="/">
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    <title>Relax NG Grammar Documentation</title>
    <link rel="stylesheet" href="relaxng.css" type="text/css"/>
  </head>
  <body>
    <h1>$X Grammar</h1>
    <div class="contents">
      <xsl:call-template name="start"/>
    </div>
  </body>
</html>
</xsl:template>
<xsl:template name="start" match="/grammar/start">
  <h2>Start Pattern: <xsl:copy-of select="./ref"/></h2>
</xsl:template>
</xsl:transform>
