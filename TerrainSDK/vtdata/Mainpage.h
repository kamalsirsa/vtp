/**
\mainpage vtdata library documentation

\section overview Overview
The <b>vtdata</b> library handles many types of geospatial data.
It is part of the <a href="http://vterrain.org/">Virtual Terrain Project</a>
and distributed under a completely free <a href="../../license.txt">open source license</a>.

\section hier Classes
<table border="1" cellpadding="7" cellspacing="0" bgcolor="#DBDBDB">
  <tr>
    <td valign="top">Built Structures
      <ul>
        <li>vtStructure</li>
        <li>vtStructInstance</li>
        <li>vtBuilding</li>
        <li>vtEdge</li>
        <li>vtLevel</li>
        <li>vtFence</li>
        <li>vtStructureArray</li>
      </ul>
      <p>Vegetation</p>
      <ul>
        <li>vtPlantDensity</li>
        <li>vtBioType</li>
        <li>vtBioRegion</li>
        <li>vtPlantAppearance</li>
        <li>vtPlantSpecies</li>
        <li>vtPlantList</li>
        <li>vtPlantInstanceArray</li>
      </ul>
      <p>Roads / Transportation</p>
      <ul>
        <li>Node</li>
        <li>Link</li>
        <li>vtRoadMap</li>
      </ul>
      <p>Vector/Polygonal Data Formats</p>
      <ul>
        <li>vtDLGFile</li>
        <li>vtLULCFile</li>
      </ul>
    </td>
    <td valign="top">
      <p>Math: Basic Types</p>
      <ul>
        <li>FPoint3 / DPoint3</li>
        <li>FPoint2 / DPoint2 / IPoint2</li>
        <li>FBox3</li>
        <li>FPlane</li>
        <li>FSphere</li>
        <li>FRECT / DRECT</li>
        <li>FMatrix3 / DMatrix3</li>
        <li>FMatrix4 / DMatrix4</li>
        <li>RGBi</li>
        <li>RGBf</li>
        <li>RGBAf</li>
        <li>CubicSpline</li>
      </ul>
      <p>Math: Array Types</p>
      <ul>
        <li>FLine2 / DLine2</li>
        <li>FLine3 / DLine3</li>
        <li>DPolygon2</li>
        <li>DPolyArray</li>
      </ul>
      <p>Miscellaneous</p>
      <ul>
        <li>vtString</li>
        <li>vtTime</li>
        <li>Triangulate_f / Triangulate_d</li>
        <li>DymaxIcosa</li>
		<li>Countries</li>
		<li>dir_iter</li>
      </ul>
    </td>
    <td valign="top">
      <p>Content</p>
      <ul>
        <li>vtContentManager</li>
        <li>vtItem</li>
        <li>vtModel</li>
        <li>vtTagArray</li>
      </ul>
      <p>Elevation</p>
      <ul>
        <li>vtHeightField</li>
        <li>vtHeightField3d</li>
        <li>vtHeightFieldGrid3d</li>
        <li>vtElevationGrid</li>
        <li>vtTin</li>
      </ul>
      <p>Bitmaps</p>
      <ul>
        <li>vtBitmapBase</li>
        <li>vtDIB</li>
      </ul>
      Projections
      <ul>
        <li>vtProjection</li>
        <li>vtLocalConversion</li>
      </ul>
      <p>Raw Features</p>
      <ul>
        <li>vtFeatureSet</li>
        <li>vtFeatureSetPoint2D</li>
        <li>vtFeatureSetPoint3D</li>
        <li>vtFeatureSetLineString</li>
        <li>vtFeatureSetPolygon</li>
      </ul>
    </td>
  </tr>
</table>
*/
