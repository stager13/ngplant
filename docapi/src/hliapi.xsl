<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

 <xsl:strip-space elements="*"/>
 <xsl:preserve-space elements="prototype"/>
 <xsl:template match="/document">
  <html>
   <head>
    <xsl:if test="count(title) > 0">
    <title><xsl:value-of select="title"/></title>
    </xsl:if>

    <style type="text/css">
      body { margin-left: 5%;
             margin-right: 2%; }

      .title { width: 100%;
               font-size: x-large;
               text-align: center }

      .secttitle { width: 100%;
                   font-size: large;
                   text-align: left;
                   clear: both }

      img.floatimg {
                    float: right;
                    margin: 0.2em;
                   }

      h3 { color: blue;
           clear: both }
      table.classes { background: #C0FFC0;
                      width : 100%;
                      border : thin solid;
                      border-color: #505050;
                      border-collapse: collapse;
                      clear: both}

      table.methods { background: #C0C0FF;
                      width : 100%;
                      border : thin solid;
                      border-color: #505050;
                      border-collapse: collapse;
                      clear: both}

      table.attributes { background: #FFC0C0;
                         width : 100%;
                         border : thin solid;
                         border-color: #505050;
                         border-collapse: collapse;
                         clear: both}

      table.stable { background: #FFFFFF;
                     width : 100%;
                     border : thin solid;
                     border-color: #505050;
                     border-collapse: collapse;
                     margin-top: 1em;
                     margin-bottom: 1em;
                     font-size: small;
                     clear: both}


      th { border : thin solid;
           text-align: left }
      td { border-left: thin solid;
           border-right:thin solid;
           margin-left : 0.5em;
           margin-right : 0.5em }

      col.key { width: 25% }

      p { text-indent: 1.8em }
      p.descpara { margin: 0.3em }

      .constant { font-weight: bolder }
      .argname { color: #0000FF }
      .ctype { font-style: italic }
      .inpre { font-family: monospace;
               font-weight: bolder}

      pre.prototype { width: 100%;
                      background: #E0E0E0;
                      padding: 0.2em }

      div.sumhead { margin: 0.5em;
                    font-size: larger;
                    font-weight: bolder;
                    clear: both }

      span.term { font-weight: bolder }

      a:link { color: blue;
               text-decoration: none }
      a:visited { color: blue;
                  text-decoration: none }

      a.classref:link { color: blue;
                        text-decoration: none }
      a.classref:visited { color: blue;
                           text-decoration: none }
    </style>
   </head>
   <body>
    <xsl:apply-templates select="title"/>
    <xsl:if test="count(section) > 0">
     <xsl:apply-templates select="section" mode="section-data"/>
    </xsl:if>
    <xsl:apply-templates select="classes" mode="classes-list"/>
    <xsl:apply-templates select="classes" mode="classes-desc"/>
   </body>
  </html>
 </xsl:template>

 <xsl:template match="title">
  <hr/>
  <div class="title"><xsl:value-of select="."/></div>
  <hr/>
 </xsl:template>

 <xsl:template match="section" mode="section-data">
  <hr/>
  <div class="secttitle"><xsl:value-of select="name"/></div>
  <xsl:apply-templates select="para"/>
 </xsl:template>

 <!-- classes-list mode -->

 <xsl:template match="classes" mode="classes-list">
 <div class="sumhead">Classes summary</div>
  <table class="classes">
   <col class="key"/><col class="value"/>
   <tr><th>Name</th><th>Short description</th></tr>
  <xsl:apply-templates select="classinfo" mode="classes-list"/>
  </table>
 </xsl:template>

 <xsl:template match="classinfo" mode="classes-list">
  <tr>
   <td>
    <a href="#Class{name}Desc"><xsl:value-of select="name"/></a>
   </td>
   <td>
    <xsl:value-of select="shortdesc"/>
   </td>
  </tr>
 </xsl:template>

 <!-- classes-desc mode -->

 <xsl:template match="classes" mode="classes-desc">
  <xsl:apply-templates select="classinfo" mode="classes-desc"/>
 </xsl:template>

 <xsl:template match="classinfo" mode="classes-desc">
  <a name="Class{name}Desc"/>
  <hr/><h3><xsl:value-of select="name"/></h3>
  <xsl:apply-templates select="desc"/>
  <xsl:apply-templates select="methods" mode="short"/>
  <xsl:apply-templates select="attributes"/>
  <xsl:apply-templates select="methods" mode="full"/>
 </xsl:template>

 <!-- mode-free -->

 <xsl:template match="desc">
  <xsl:apply-templates/>
 </xsl:template>

 <!-- short mode -->

 <xsl:template match="methods" mode="short">
 <div class="sumhead">Methods summary</div>
  <table class="methods">
  <col class="key"/><col class="value"/>
  <tr><th>Name</th><th>Short description</th></tr>
  <xsl:for-each select="methodinfo">
   <tr>
    <td>
     <xsl:if test="desc">
      <a href="#Method{../../name}{name}Desc"><xsl:value-of select="name"/></a>
     </xsl:if>
     <xsl:if test="count(desc) = 0">
      <xsl:value-of select="name"/>
     </xsl:if>
    </td>
    <td>
     <xsl:value-of select="shortdesc"/>
    </td>
   </tr>
  </xsl:for-each>
  </table>
 </xsl:template>

 <xsl:template match="methods" mode="full">
  <xsl:for-each select="methodinfo">
   <xsl:if test="desc">
    <hr/>
    <a name="Method{../../name}{name}Desc"/>
    <div class="sumhead">Method <xsl:value-of select="name"/></div>
    <xsl:apply-templates select="prototype"/>
    <xsl:apply-templates select="desc"/>
   </xsl:if>
  </xsl:for-each>
 </xsl:template>

 <xsl:template match="attributes">
   <xsl:if test="count(attributeinfo) > 0">
    <div class="sumhead">Attributes summary</div>
    <table class="attributes">
     <col class="key"/><col class="value"/>
     <tr><th>Name</th><th>Short description</th></tr>
     <xsl:for-each select="attributeinfo">
      <tr>
       <td>
        <xsl:value-of select="name"/>
       </td>
       <td>
        <xsl:value-of select="shortdesc"/>
       </td>
      </tr>
     </xsl:for-each>
    </table>
   </xsl:if>
 </xsl:template>

 <xsl:template match="constant">
  <span class="constant"><xsl:value-of select="."/></span>
 </xsl:template>

 <xsl:template match="argname">
  <span class="argname"><xsl:value-of select="."/></span>
 </xsl:template>

 <xsl:template match="prototype">
  <pre class="prototype"><xsl:value-of select="."/></pre>
 </xsl:template>

 <xsl:template match="ctype">
  <span class="ctype"><xsl:value-of select="."/></span>
 </xsl:template>

 <xsl:template match="inpre">
  <span class="inpre"><xsl:value-of select="."/></span>
 </xsl:template>

 <xsl:template match="term">
  <span class="term"><xsl:value-of select="."/></span>
 </xsl:template>

 <xsl:template match="classref">
  <a class="classref" href="#Class{@classname}Desc"><xsl:value-of select="@classname"/></a>
 </xsl:template>

 <xsl:template match="methodref">
  <a class="classref" href="#Method{@classname}{@name}Desc"><xsl:value-of select="@name"/></a>
 </xsl:template>

 <xsl:template match="para">
  <p class="descpara"><xsl:apply-templates/></p>
 </xsl:template>

 <xsl:template match="stable">
  <table class="stable">
   <xsl:apply-templates/>
  </table>
 </xsl:template>

 <xsl:template match="str">
  <tr>
   <xsl:apply-templates/>
  </tr>
 </xsl:template>

 <xsl:template match="sth">
  <th>
   <xsl:apply-templates/>
  </th>
 </xsl:template>

 <xsl:template match="std">
  <td>
   <xsl:apply-templates/>
  </td>
 </xsl:template>

 <xsl:template match="ul">
  <ul>
   <xsl:apply-templates/>
  </ul>
 </xsl:template>

 <xsl:template match="li">
  <li><xsl:apply-templates/></li>
 </xsl:template>

 <xsl:template match="figure">
  <div><img class="floatimg" src="{image/@ref}"/></div>
 </xsl:template>

</xsl:stylesheet>

