--[[

ngPlant-plugin : model-export
menu-name : COLLADA (.dae) (plug-in)

--]]

local InstanceTransformModeNone     = 0
local InstanceTransformModeSketchUp = 1

local InstanceTransformMode    = InstanceTransformModeSketchUp
local EmulateTwoSidedMaterials = false

local MATH_PI = 3.14159265358979
local XmlNestLevel = 0

local function GetISO8601Time ()
  --FIXME: timezone is not used

  local d = os.date("%FT%TZ")

  --FIXME: in some circumstances %F and %T fields are ignored under
  --       Windows. In such cases predefined value is used
  if d == "TZ" then
    return "2013-02-14T23:45:00Z"
  else
    return d
  end
end

local function QuaternionNormalize (Quat)
  local X = Quat[1]
  local Y = Quat[2]
  local Z = Quat[3]
  local W = Quat[4]

  local Len = math.sqrt(X * X + Y * Y + Z * Z + W * W)

  return { X / Len, Y / Len, Z / Len, W / Len }
end

local function QuaternionToAxisAndAngle (Quat)
  local Q     = QuaternionNormalize(Quat)
  local CosA  = Q[4]
  local SinA  = math.sqrt(1.0 - CosA * CosA)
  local Angle = math.acos(CosA) * 2.0

  if SinA > -0.000001 and SinA < 0.000001 then
    SinA = 1.0
  end

  return Q[1] / SinA,Q[2] / SinA,Q[3] / SinA,Angle
end

local function XmlBeginElement (File,Tag,Attrs,Empty,ContinueLine)
  File:write(string.rep(" ",XmlNestLevel))
  File:write("<"..Tag)

  if Attrs then
    for name,val in pairs(Attrs) do
      File:write(" "..name.."=\""..val.."\"")
    end
  end

  if Empty then
   File:write("/>\n")
  else
    if ContinueLine then
      File:write(">")
    else
      File:write(">\n")
    end
   XmlNestLevel = XmlNestLevel + 1
  end
end

local function XmlEndElement (File,Tag)
  XmlNestLevel = XmlNestLevel - 1

  File:write(string.rep(" ",XmlNestLevel))
  File:write("</"..Tag..">\n")
end

local function XmlElement (File,Tag,Attrs,Data)
 if Data then
  XmlBeginElement(File,Tag,Attrs,false,true)
  XmlNestLevel = XmlNestLevel - 1
  File:write(Data)
  File:write("</"..Tag..">\n")
 else
  XmlBeginElement(File,Tag,Attrs,true)
 end
end

local function GetGeometryId (Group)
  return Group:GetName()
end

local function GetGeometryName (Group)
  return GetGeometryId(Group)
end

local function GetSourcePosId (Group)
  return Group:GetName() .. "-pos"
end

local function GetSourcePosArrayId (Group)
  return Group:GetName() .. "-pos-array"
end

local function GetSourceNormalId (Group)
  return Group:GetName() .. "-normal"
end

local function GetSourceNormalArrayId (Group)
  return Group:GetName() .. "-normal-array"
end

local function GetSourceTexCoordId (Group)
  return Group:GetName() .. "-texcoord"
end

local function GetSourceTexCoordArrayId (Group)
  return Group:GetName() .. "-texcoord-array"
end

local function GetVerticesId (Group)
  return Group:GetName() .. "-vertices"
end

local function GetNodeId (Group)
  return Group:GetName() .. "-node"
end

local function GetNodeName (Group)
  return GetNodeId(Group)
end

local function GetEffectId (Group)
  return Group:GetName() .. "-fx"
end

local function GetEffectSId (Group)
  return GetEffectId(Group) .. "-sid"
end

local function GetMaterialId (Group)
  return Group:GetName() .. "-mt"
end

local function GetMaterialSymbol (Group)
  return GetMaterialId(Group) .. "-sym"
end

local function GetDiffImageId (Group)
  return GetMaterialId(Group) .. "-diffuse-img"
end

local function GetSurfaceParamId (Group)
  return GetMaterialId(Group) .. "-surface"
end

local function GetSampler2DParamId (Group)
  return GetMaterialId(Group) .. "-sampler2d"
end

local function GetTexCoordId  (Group)
  return GetMaterialId(Group) .. "-uvs"
end

local function WriteFloatArrayBegin (F,ArrayId,AttrCount,AttrSize)
  XmlBeginElement(F,"float_array",{ id = ArrayId, count = AttrCount * AttrSize })
end

local function WriteFloatArrayData (F,Buffer,MapFn)
  local AttrCount = table.getn(Buffer)
  local Indent    = string.rep(" ",XmlNestLevel)

  if AttrCount > 0 then
    local AttrSize = table.getn(Buffer[1])

    if AttrSize == 2 then
      for i,v in ipairs(Buffer) do
        if MapFn then
          v = MapFn(v)
        end
        F:write(Indent)
        F:write(string.format("%f %f\n",v[1],v[2]))
      end
    else
      for i,v in ipairs(Buffer) do
        if MapFn then
          v = MapFn(v)
        end
        F:write(Indent)
        F:write(string.format("%f %f %f\n",v[1],v[2],v[3]))
      end
    end
  end
end

local function WriteFloatArrayEnd (F)
  XmlEndElement(F,"float_array")
end

local function WriteFloatArray (F,ArrayId,Buffer,AttrSize,MapFn)
  WriteFloatArrayBegin(F,ArrayId,table.getn(Buffer),AttrSize)
  WriteFloatArrayData(F,Buffer,MapFn)
  WriteFloatArrayEnd(F)
end

local function GetPolyVertexCount (Group,Index)
  if Group:GetPrimitiveType(Index) == NGP_QUAD then
    return 4
  else
    return 3
  end
end

local function WriteVertexCountArray (F,Group,PrimitiveCount,RepeatCount)
  local Indent = string.rep(" ",XmlNestLevel)

  for RepeatIndex = 1,RepeatCount do
    if RepeatIndex > 1 then
      F:write("\n")
    end

    for PrimitiveIndex = 1,PrimitiveCount do
      if PrimitiveIndex % 10 == 1 then
        if PrimitiveIndex > 1 then
          F:write("\n")
        end
       F:write(Indent)
      end

      F:write(" " .. GetPolyVertexCount(Group,PrimitiveIndex))
    end
  end

  F:write("\n")
end

local function ExportLibraryImages(F)
  local HasImages = false

  for GroupIndex = 1,PlantModel:GetGroupCount() do
    local Group = PlantModel:GetGroup(GroupIndex)
    local Material = Group:GetMaterial()
    local Texture  = Material.TexNames[NGP_TEX_DIFFUSE]

    if Texture then
      if not HasImages then
        HasImages = true
        XmlBeginElement(F,"library_images")
      end

      XmlBeginElement(F,"image",{ id = GetDiffImageId(Group) })
       XmlElement(F,"init_from",nil,Texture)
      XmlEndElement(F,"image")
    end
  end

  if HasImages then
    XmlEndElement(F,"library_images")
  end
end

local function ExportLibraryEffects(F)
  XmlBeginElement(F,"library_effects")

   for GroupIndex = 1,PlantModel:GetGroupCount() do
     local Group = PlantModel:GetGroup(GroupIndex)
     local Material = Group:GetMaterial()
     local Texture  = Material.TexNames[NGP_TEX_DIFFUSE]
     local Color    = Material.Color
     local ColorStr = string.format("%f %f %f 1.0",Color.R,Color.G,Color.B)

     XmlBeginElement(F,"effect",{ id = GetEffectId(Group) } )
      XmlBeginElement(F,"profile_COMMON")

       if Texture then
         XmlBeginElement(F,"newparam",{ sid = GetSurfaceParamId(Group) })
          XmlBeginElement(F,"surface",{ ["type"] = "2D"})
           XmlElement(F,"init_from",nil,GetDiffImageId(Group))
          XmlEndElement(F,"surface")
         XmlEndElement(F,"newparam")

         XmlBeginElement(F,"newparam",{ sid = GetSampler2DParamId(Group) })
          XmlBeginElement(F,"sampler2D")
           XmlElement(F,"source",nil,GetSurfaceParamId(Group))
          XmlEndElement(F,"sampler2D")
         XmlEndElement(F,"newparam")
       end

       XmlBeginElement(F,"technique",{ sid = GetEffectSId(Group) })
        XmlBeginElement(F,"phong")
         XmlBeginElement(F,"emission")
          XmlElement(F,"color",nil,"0.0 0.0 0.0 1.0")
         XmlEndElement(F,"emission")
         XmlBeginElement(F,"ambient")
          XmlElement(F,"color",nil,ColorStr)
         XmlEndElement(F,"ambient")
         XmlBeginElement(F,"diffuse")

          if Texture then
            XmlElement(F,"texture",{texture=GetSampler2DParamId(Group),
                                    texcoord=GetTexCoordId(Group)})
          else
            XmlElement(F,"color",nil,ColorStr)
          end

         XmlEndElement(F,"diffuse")
         XmlBeginElement(F,"specular")
          XmlElement(F,"color",nil,"0.0 0.0 0.0 1.0")
         XmlEndElement(F,"specular")
         XmlBeginElement(F,"shininess")
          XmlElement(F,"float",nil,"0.0")
         XmlEndElement(F,"shininess")
         XmlBeginElement(F,"reflective")
          XmlElement(F,"color",nil,"0.0 0.0 0.0 1.0")
         XmlEndElement(F,"reflective")
         XmlBeginElement(F,"reflectivity")
          XmlElement(F,"float",nil,"0.0")
         XmlEndElement(F,"reflectivity")
         XmlBeginElement(F,"transparent")
          XmlElement(F,"color",nil,"0.0 0.0 0.0 1.0")
         XmlEndElement(F,"transparent")
         XmlBeginElement(F,"transparency")
          XmlElement(F,"float",nil,"1.0")
         XmlEndElement(F,"transparency")
        XmlEndElement(F,"phong")
       XmlEndElement(F,"technique")
      XmlEndElement(F,"profile_COMMON")
     XmlEndElement(F,"effect")
   end

  XmlEndElement(F,"library_effects")
end

local function ExportLibraryMaterials(F)
  XmlBeginElement(F,"library_materials")

   for GroupIndex = 1,PlantModel:GetGroupCount() do
     local Group = PlantModel:GetGroup(GroupIndex)

     XmlBeginElement(F,"material",{ id = GetMaterialId(Group) })
      XmlElement(F,"instance_effect", { url = "#" .. GetEffectId(Group) } )
     XmlEndElement(F,"material")
   end

  XmlEndElement(F,"library_materials")
end

local function GetVAttrBufferForGroup(Group,Attr)
  if Group:IsCloneable() then
    return Group:GetCloneVAttrBuffer(Attr)
  else
    return Group:GetVAttrBuffer(Attr)
  end
end

local function ExportLibraryGeometry(F)
 XmlBeginElement(F,"library_geometries")

 for GroupIndex = 1,PlantModel:GetGroupCount() do
   local Group          = PlantModel:GetGroup(GroupIndex)
   local TwoSided       = Group:GetMaterial().DoubleSided
   local DuplicateFaces = TwoSided and EmulateTwoSidedMaterials

   XmlBeginElement(F,"geometry",{ id   = GetGeometryId(Group),
                                  name = GetGeometryName(Group)} )
    XmlBeginElement(F,"mesh")
     XmlBeginElement(F,"source",{ id = GetSourcePosId(Group) })

      local PosBuffer = GetVAttrBufferForGroup(Group,NGP_ATTR_VERTEX)

      WriteFloatArray(F,GetSourcePosArrayId(Group),PosBuffer,3)

      XmlBeginElement(F,"technique_common")
       XmlBeginElement(F,"accessor",{ source = "#"..GetSourcePosArrayId(Group),
                                      count  = table.getn(PosBuffer),
                                      stride = 3 })
        XmlElement(F,"param",{ name = "X", ["type"] = "float" })
        XmlElement(F,"param",{ name = "Y", ["type"] = "float" })
        XmlElement(F,"param",{ name = "Z", ["type"] = "float" })
       XmlEndElement(F,"accessor")
      XmlEndElement(F,"technique_common")
      PosBuffer = nil
     XmlEndElement(F,"source")

     XmlBeginElement(F,"source",{ id = GetSourceNormalId(Group) })
      local NormalBuffer = GetVAttrBufferForGroup(Group,NGP_ATTR_NORMAL)
      local NormalAttrCount = table.getn(NormalBuffer)

      if DuplicateFaces then
        WriteFloatArrayBegin(F,GetSourceNormalArrayId(Group),NormalAttrCount * 2,3)
      else
        WriteFloatArrayBegin(F,GetSourceNormalArrayId(Group),NormalAttrCount,3)
      end

      WriteFloatArrayData(F,NormalBuffer)

      if DuplicateFaces then
        WriteFloatArrayData(F,NormalBuffer,
                            function(v) return {-v[1],-v[2],-v[3]} end)
      end

      WriteFloatArrayEnd(F)

      XmlBeginElement(F,"technique_common")
       XmlBeginElement(F,"accessor",{ source = "#"..GetSourceNormalArrayId(Group),
                                      count  = table.getn(NormalBuffer),
                                      stride = 3 })
        XmlElement(F,"param",{ name = "X", ["type"] = "float" })
        XmlElement(F,"param",{ name = "Y", ["type"] = "float" })
        XmlElement(F,"param",{ name = "Z", ["type"] = "float" })
       XmlEndElement(F,"accessor")
      XmlEndElement(F,"technique_common")
      NormalBuffer = nil
     XmlEndElement(F,"source")

     XmlBeginElement(F,"source",{ id = GetSourceTexCoordId(Group) })
      local TexCoordBuffer = GetVAttrBufferForGroup(Group,NGP_ATTR_TEXCOORD0)

      WriteFloatArray(F,GetSourceTexCoordArrayId(Group),TexCoordBuffer,2)

      XmlBeginElement(F,"technique_common")
       XmlBeginElement(F,"accessor",{ source = "#"..GetSourceTexCoordArrayId(Group),
                                      count  = table.getn(TexCoordBuffer),
                                      stride = 2 })
        XmlElement(F,"param",{ name = "S", ["type"] = "float" })
        XmlElement(F,"param",{ name = "T", ["type"] = "float" })
       XmlEndElement(F,"accessor")
      XmlEndElement(F,"technique_common")
      TexCoordBuffer = nil
     XmlEndElement(F,"source")

     XmlBeginElement(F,"vertices",{ id = GetVerticesId(Group) })
      XmlElement(F,"input",{ semantic = "POSITION" , source = "#"..GetSourcePosId(Group) })
     XmlEndElement(F,"vertices")

     local IsCloneable         = Group:IsCloneable()
     local VertexIndexBuffer   = Group:GetVAttrIndexBuffer(NGP_ATTR_VERTEX,not IsCloneable)
     local NormalIndexBuffer   = Group:GetVAttrIndexBuffer(NGP_ATTR_NORMAL,not IsCloneable)
     local TexCoordIndexBuffer = Group:GetVAttrIndexBuffer(NGP_ATTR_TEXCOORD0,not IsCloneable)

     local PrimitiveCount = table.getn(VertexIndexBuffer)

     XmlBeginElement(F,"polylist",{ count = PrimitiveCount, material = GetMaterialSymbol(Group) })
      XmlElement(F,"input",{ offset = "0", semantic = "VERTEX",   source = "#"..GetVerticesId(Group) })
      XmlElement(F,"input",{ offset = "1", semantic = "NORMAL",   source = "#"..GetSourceNormalId(Group) })
      XmlElement(F,"input",{ offset = "2", semantic = "TEXCOORD", source = "#"..GetSourceTexCoordId(Group) })
      XmlBeginElement(F,"vcount")
       if DuplicateFaces then
         WriteVertexCountArray(F,Group,PrimitiveCount,2)
       else
         WriteVertexCountArray(F,Group,PrimitiveCount,1)
       end
      XmlEndElement(F,"vcount")
      XmlBeginElement(F,"p")

      local PosIndex
      local NormalIndex
      local TexCoordIndex
      local Indent = string.rep(" ",XmlNestLevel)

      for PrimitiveIndex = 1,PrimitiveCount do
         PosIndex      = VertexIndexBuffer[PrimitiveIndex]
         NormalIndex   = NormalIndexBuffer[PrimitiveIndex]
         TexCoordIndex = TexCoordIndexBuffer[PrimitiveIndex]

         F:write(Indent)

         for VertexIndex = 1,GetPolyVertexCount(Group,PrimitiveIndex) do
           F:write(" " .. PosIndex[VertexIndex]-1 .. " " ..
                   NormalIndex[VertexIndex]-1 .. " " ..
                   TexCoordIndex[VertexIndex]-1)
         end

         F:write("\n")
      end

      if DuplicateFaces then
        for PrimitiveIndex = 1,PrimitiveCount do
           PosIndex      = VertexIndexBuffer[PrimitiveIndex]
           NormalIndex   = NormalIndexBuffer[PrimitiveIndex]
           TexCoordIndex = TexCoordIndexBuffer[PrimitiveIndex]

           F:write(Indent)

           local PolyVertexCount = GetPolyVertexCount(Group,PrimitiveIndex)

           for VertexIndex = PolyVertexCount,1,-1 do
             F:write(" " .. PosIndex[VertexIndex]-1 .. " " ..
                     NormalAttrCount + NormalIndex[VertexIndex]-1 .. " " ..
                     TexCoordIndex[VertexIndex]-1)
           end

           F:write("\n")
        end
      end

      XmlEndElement(F,"p")
     XmlEndElement(F,"polylist")

    XmlEndElement(F,"mesh")
   XmlEndElement(F,"geometry")
 end

 XmlEndElement(F,"library_geometries")
end

local function ExportLibraryNodes(F)
 XmlBeginElement(F,"library_nodes")
  for GroupIndex = 1,PlantModel:GetGroupCount() do
    local Group = PlantModel:GetGroup(GroupIndex)

    XmlBeginElement(F,"node",{ id = GetNodeId(Group), name = GetNodeName(Group)})
      XmlBeginElement(F,"instance_geometry",{ url = "#" .. GetGeometryId(Group)})
       XmlBeginElement(F,"bind_material")
        XmlBeginElement(F,"technique_common")
         XmlBeginElement(F,"instance_material",
                           { symbol = GetMaterialSymbol(Group),
                            target = "#" .. GetMaterialId(Group) })
          if Group:GetMaterial().TexNames[NGP_TEX_DIFFUSE] then
            XmlElement(F,"bind_vertex_input",{semantic=GetTexCoordId(Group),
                                              input_semantic="TEXCOORD"})
          end
         XmlEndElement(F,"instance_material")
        XmlEndElement(F,"technique_common")
       XmlEndElement(F,"bind_material")
      XmlEndElement(F,"instance_geometry")
    XmlEndElement(F,"node")
  end
 XmlEndElement(F,"library_nodes")
end

local function ExportVisualScenes(F)
 XmlBeginElement(F,"library_visual_scenes")
  XmlBeginElement(F,"visual_scene",{ id = "DefaultScene" })

   for GroupIndex = 1,PlantModel:GetGroupCount() do
     local Group = PlantModel:GetGroup(GroupIndex)

     if Group:IsCloneable() then
       Translations,Orientations = Group:GetCloneTransformBuffer()

       local CloneCount = table.getn(Translations)

       for CloneIndex = 1,CloneCount do
         XmlBeginElement(F,"node",{ id   = GetNodeId(Group) .. "-sc" .. string.format("%d",CloneIndex),
                                    name = GetNodeName(Group) .. "-sc" .. string.format("%d",CloneIndex)})
           local T = Translations[CloneIndex]
           local O = Orientations[CloneIndex]
           local X,Y,Z,A

           X,Y,Z,A = QuaternionToAxisAndAngle(O)

           if InstanceTransformMode == InstanceTransformModeSketchUp then
             T[2],T[3] = -T[3],T[2]
             Y,Z       = -Z,Y
           end

           XmlElement(F,"translate",nil,string.format("%f %f %f",T[1],T[2],T[3]))
           XmlElement(F,"rotate",nil,string.format("%f %f %f %f",X,Y,Z,A * 180.0 / MATH_PI))
           XmlElement(F,"instance_node",{ url = "#" .. GetNodeId(Group)})
         XmlEndElement(F,"node")
       end
     else
       XmlBeginElement(F,"node",{ id   = GetNodeId(Group) .. "-sc",
                                  name = GetNodeName(Group) .. "-sc"})
         XmlElement(F,"instance_node",{ url = "#" .. GetNodeId(Group)})
       XmlEndElement(F,"node")
     end
   end

  XmlEndElement(F,"visual_scene")
 XmlEndElement(F,"library_visual_scenes")
end

local function Export(FileName)
 local F = io.open(FileName,"w")

 F:write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")

 XmlBeginElement(F,"COLLADA",
                 { xmlns = "http://www.collada.org/2005/11/COLLADASchema",
                   version = "1.4.1" })

  local CurrentTime = GetISO8601Time()

  XmlBeginElement(F,"asset")
   XmlElement(F,"created",nil,CurrentTime)
   XmlElement(F,"modified",nil,CurrentTime)
   XmlElement(F,"up_axis",nil,"Y_UP")
  XmlEndElement(F,"asset")

  if PlantModel:GetGroupCount() > 0 then
    ExportLibraryImages(F)
    ExportLibraryEffects(F)
    ExportLibraryMaterials(F)
    ExportLibraryGeometry(F)
    ExportLibraryNodes(F)
    ExportVisualScenes(F)

    XmlBeginElement(F,"scene")
     XmlElement(F,"instance_visual_scene",{ url = "#DefaultScene" } )
    XmlEndElement(F,"scene")
  end

 XmlEndElement(F,"COLLADA")

 F:close()
end

FileName = ShowFileSaveDialog("Choose .dae file name")

if FileName then
  Export(FileName)
end

