<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0">

  <xsl:import href="/usr/share/sgml/docbook/stylesheet/xsl/nwalsh/html/docbook.xsl"/>

  <!-- No ToC or chunks -->
  <xsl:param name="generate.toc">
  </xsl:param>
  <xsl:param name="using.chunker" select="0"/>

  <!-- Add email to author's name -->
  <xsl:template match="author" mode="titlepage.mode">
    <div class="{name(.)}">
      <h3 class="{name(.)}">
        <xsl:call-template name="person.name"/>
	<xsl:text> &lt;</xsl:text>
	<xsl:value-of select="./email"/>
	<xsl:text>&gt;</xsl:text>
      </h3>
      <xsl:apply-templates mode="titlepage.mode" select="./contrib"/>
      <xsl:apply-templates mode="titlepage.mode" select="./affiliation"/>
    </div>
  </xsl:template>
</xsl:stylesheet>
