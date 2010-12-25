<?xml version='1.0'?>
<xsl:stylesheet
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/docbook.xsl"/>

<xsl:param name="html.stylesheet" select="'script.css'"/>

<xsl:output method="html"
            indent="yes"/>

<!--<xsl:param name="variablelist.as.table" select="1"/>-->

  <xsl:template match="sect1">
    <xsl:if test="preceding-sibling::sect1">
      <hr/>
    </xsl:if>
    <xsl:apply-imports/>
  </xsl:template>

<!--  <xsl:template match="sect2">
    <xsl:if test="preceding-sibling::sect2">
      <hr/>
    </xsl:if>
    <xsl:apply-imports/>
  </xsl:template>-->

</xsl:stylesheet>
