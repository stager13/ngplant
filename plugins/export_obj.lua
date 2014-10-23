--[[

ngPlant-plugin : model-export
menu-name : Alias-Wavefront (.OBJ) (plug-in)

--]]

--[[
                   ====== BEGIN GPL LICENSE BLOCK =====

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

                   ===== END GPL LICENSE BLOCK =====
--]]

local function VisibleGroupsIter(Model)
 local IntGroupIndex = 0
 local VisGroupIndex = 0
 local GroupCount    = Model:GetGroupCount()

 return function ()
         local LODLevel = GetCurrentLOD()

         IntGroupIndex = IntGroupIndex + 1

         while IntGroupIndex <= GroupCount do
          local Group = Model:GetGroup(IntGroupIndex)
          local MinLOD,MaxLOD

          MinLOD,MaxLOD = Group:GetLODVisRange()

          if (ExportHiddenGroups or (not Group:IsHidden())) and
             (ExportOutVisRangeGroups or ((LODLevel >= MinLOD) and (LODLevel <= MaxLOD))) then
           VisGroupIndex = VisGroupIndex + 1

           return VisGroupIndex,Group
          else
           IntGroupIndex = IntGroupIndex + 1
          end
         end

         return nil
        end
end

local function IsSimilarColorComponent(C1,C2)
 return math.abs(C1 - C2) < (1 / 256)
end

local function IsSimilarMaterials(Mat1,Mat2)
 return IsSimilarColorComponent(Mat1.Color.R,Mat2.Color.R) and
        IsSimilarColorComponent(Mat1.Color.G,Mat2.Color.G) and
        IsSimilarColorComponent(Mat1.Color.B,Mat2.Color.B) and
        Mat1.TexNames[NGP_TEX_DIFFUSE] == Mat2.TexNames[NGP_TEX_DIFFUSE]
end

local function GetSimilarMatIndex(Material)
 for GroupIndex,Group in VisibleGroupsIter(PlantModel) do
  local CurrentMaterial = Group:GetMaterial()

  if IsSimilarMaterials(Material,Group:GetMaterial()) then
   return GroupIndex
  end
 end

 error('assertion failed: material can not be found')
end

local function CreateMaterialsMapping(JoinMaterials)
 local Mapping = {}

 if not JoinMaterials then
  for i = 1,PlantModel:GetGroupCount() do
   Mapping[i] = i
  end

  return Mapping
 end

 for GroupIndex,Group in VisibleGroupsIter(PlantModel) do
  Mapping[GroupIndex] = GetSimilarMatIndex(Group:GetMaterial())
 end

 return Mapping
end

local function ExportOBJFile(OBJFileName,MTLFileName,MaterialsMapping,ApplyScaling,ScalingFactor)
 local OBJFile = io.open(OBJFileName,"w")

 OBJFile:write("o plant\n")
 OBJFile:write("mtllib " .. MTLFileName .. "\n")

 local VertexIndexOffset   = 1
 local NormalIndexOffset   = 1
 local TexCoordIndexOffset = 1

 for GroupIndex,Group in VisibleGroupsIter(PlantModel) do
  local Material = Group:GetMaterial()

  OBJFile:write(string.format("g bgroup%s\n",GroupIndex))
  OBJFile:write(string.format("usemtl pmat%s\n",MaterialsMapping[GroupIndex]))

  if Material.TexNames[NGP_TEX_DIFFUSE] then
   OBJFile:write(string.format("usemap %s\n",GetTextureFileName(Material.TexNames[NGP_TEX_DIFFUSE])))
  else
   OBJFile:write("usemap off\n")
  end

  local Buffer = Group:GetVAttrBuffer(NGP_ATTR_VERTEX)

  local VertexIndexStep = table.getn(Buffer)

  if ApplyScaling then
   for i,v in ipairs(Buffer) do
    OBJFile:write(string.format("v %f %f %f\n",v[1] * ScalingFactor,v[2] * ScalingFactor,v[3] * ScalingFactor))
   end
  else
   for i,v in ipairs(Buffer) do
    OBJFile:write(string.format("v %f %f %f\n",v[1],v[2],v[3]))
   end
  end

  Buffer = Group:GetVAttrBuffer(NGP_ATTR_NORMAL)

  local NormalIndexStep = table.getn(Buffer)

  for i,v in ipairs(Buffer) do
   OBJFile:write(string.format("vn %f %f %f\n",v[1],v[2],v[3]))
  end

  Buffer = Group:GetVAttrBuffer(NGP_ATTR_TEXCOORD0)

  local TexCoordIndexStep = table.getn(Buffer)

  for i,v in ipairs(Buffer) do
   OBJFile:write(string.format("vt %f %f\n",v[1],v[2]))
  end

  Buffer = nil

  local VertexIndexBuffer   = Group:GetVAttrIndexBuffer(NGP_ATTR_VERTEX,true,VertexIndexOffset)
  local NormalIndexBuffer   = Group:GetVAttrIndexBuffer(NGP_ATTR_NORMAL,true,NormalIndexOffset)
  local TexCoordIndexBuffer = Group:GetVAttrIndexBuffer(NGP_ATTR_TEXCOORD0,true,TexCoordIndexOffset)

  local PrimitiveCount = table.getn(VertexIndexBuffer)

  for PrimitiveIndex = 1,PrimitiveCount do
   local vi = VertexIndexBuffer[PrimitiveIndex]
   local ni = NormalIndexBuffer[PrimitiveIndex]
   local ti = TexCoordIndexBuffer[PrimitiveIndex]

   if Group:GetPrimitiveType(PrimitiveIndex) == NGP_QUAD then
    OBJFile:write(string.format("f %u/%u/%u %u/%u/%u %u/%u/%u %u/%u/%u\n",
                                 vi[1],ti[1],ni[1],
                                 vi[2],ti[2],ni[2],
                                 vi[3],ti[3],ni[3],
                                 vi[4],ti[4],ni[4]))
   else
    OBJFile:write(string.format("f %u/%u/%u %u/%u/%u %u/%u/%u\n",
                                 vi[1],ti[1],ni[1],
                                 vi[2],ti[2],ni[2],
                                 vi[3],ti[3],ni[3]))
   end
  end

  VertexIndexOffset   = VertexIndexOffset   + VertexIndexStep
  NormalIndexOffset   = NormalIndexOffset   + NormalIndexStep
  TexCoordIndexOffset = TexCoordIndexOffset + TexCoordIndexStep

 end

 OBJFile:close()
end

local function ExportMTLFile(MTLFileName,MaterialsMapping,ExportDir)
 local MTLFile = io.open(MTLFileName,"w")
 local TexFileList = {}

 for GroupIndex,Group in VisibleGroupsIter(PlantModel) do
  if MaterialsMapping[GroupIndex] == GroupIndex then
   local Material = Group:GetMaterial()

   MTLFile:write(string.format("newmtl pmat%u\n",GroupIndex))
   MTLFile:write(string.format("Kd %f %f %f\n",Material.Color.R,Material.Color.G,Material.Color.B))
   MTLFile:write(string.format("Ns 1\n"))

   local TexName = Material.TexNames[NGP_TEX_DIFFUSE]

   if TexName then
    if ExportDir then
     TexName     = OSPathBaseName(TexName)
     TexFileName = Material.TexFileNames[NGP_TEX_DIFFUSE]

     if TexFileList[TexName] then
      if TexFileList[TexName] ~= TexFileName then
       error("Several textures with the same basename (" .. TexName .. " are not allowed")
      end
     else
      local DestFileName = OSPathJoin(ExportDir,TexName)

      if not OSFileExists(DestFileName) or
         ShowYesNoMessageBox("File " .. DestFileName .. " already exists. Overwrite it?","Confirmation") then
       OSFileCopy(TexFileName,DestFileName)
      end
     end
    else
     TexName = GetTextureFileName(TexName)
    end

    MTLFile:write(string.format("map_Kd %s\n",TexName))
   end
  end
 end

 MTLFile:close()
end

MinX,MinY,MinZ,MaxX,MaxY,MaxZ = PlantModel:GetBoundingBox()
OriginalHeight = MaxY - MinY

DialogDescription =
 {
  {
   label   = "Create self-contained dir",
   name    = "CreateSelfContainedDir",
   type    = "choice",
   choices = { "Yes","No" },
   default = 0
  },
  {
   label   = "Export hidden branch groups",
   name    = "ExportHiddenGroups",
   type    = "choice",
   choices = { "Yes","No" },
   default = ExportPreferences.HiddenGroupsExportMode == NGP_NEVER and 1 or 0
  },
  {
   label   = "Export branches outside visible range",
   name    = "ExportOutVisRangeGroups",
   type    = "choice",
   choices = { "Yes","No" },
   default = ExportPreferences.OutVisRangeExportMode == NGP_NEVER and 1 or 0
  },
  {
   label   = "Join similar materials",
   name    = "JoinSimilarMaterials",
   type    = "choice",
   choices = { "Yes","No" },
   default = 0
  },
  {
   label   = "Apply scaling",
   name    = "ApplyScaling",
   type    = "choice",
   choices = { "No","Yes" },
   default = 0
  },
  {
   label   = "Scale to height",
   name    = "ScaledHeight",
   type    = "number",
   default = OriginalHeight
  }
 }

Params = ShowParameterDialog(DialogDescription)

if Params then
 ExportHiddenGroups      = Params.ExportHiddenGroups == "Yes"
 ExportOutVisRangeGroups = Params.ExportOutVisRangeGroups == "Yes"

 local JoinMaterials           = Params.JoinSimilarMaterials == "Yes"
 local CreateSelfContainedDir  = Params.CreateSelfContainedDir == "Yes"

 local ApplyScaling  = Params.ApplyScaling
 local ScalingFactor = Params.ScaledHeight / OriginalHeight

 if CreateSelfContainedDir then
  local DirName = ShowDirSelectDialog("Choose directory")

  if DirName then
   local OBJFileName = ShowGetStringDialog("Choose .obj file name","",GetDerivedFileName("obj"))

   if OBJFileName then
    local MTLFileName = string.gsub(OBJFileName,".[oO][bB][jJ]$",".mtl")

    local FullOBJFileName = OSPathJoin(DirName,OBJFileName)
    local FullMTLFileName = OSPathJoin(DirName,MTLFileName)

    local MaterialsMapping = CreateMaterialsMapping(JoinMaterials)

    ExportOBJFile(FullOBJFileName,MTLFileName,MaterialsMapping,ApplyScaling,ScalingFactor)
    ExportMTLFile(FullMTLFileName,MaterialsMapping,DirName)
   end
  end
 else
  local FullOBJFileName = ShowFileSaveDialog("Choose .obj file name",GetDerivedFileName("obj"))

  if FullOBJFileName then
   local FullMTLFileName = string.gsub(FullOBJFileName,".[oO][bB][jJ]$",".mtl")

   local MaterialsMapping = CreateMaterialsMapping(JoinMaterials)

   ExportOBJFile(FullOBJFileName,FullMTLFileName,MaterialsMapping)
   ExportMTLFile(FullMTLFileName,MaterialsMapping)
  end
 end
end

