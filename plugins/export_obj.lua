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

function VisibleGroupsIter(Model)
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

function ExportOBJFile(OBJFileName,MTLFileName)
 OBJFile = io.open(OBJFileName,"w")

 OBJFile:write("o plant\n")
 OBJFile:write("mtllib " .. MTLFileName .. "\n")

 VertexIndexOffset   = 1
 NormalIndexOffset   = 1
 TexCoordIndexOffset = 1

 for GroupIndex,Group in VisibleGroupsIter(PlantModel) do
  Material = Group:GetMaterial()

  OBJFile:write(string.format("usemtl pmat%s\n",GroupIndex))

  if Material.TexNames[NGP_TEX_DIFFUSE] then
   OBJFile:write(string.format("usemap %s\n",GetTextureFileName(Material.TexNames[NGP_TEX_DIFFUSE])))
  else
   OBJFile:write("usemap off\n")
  end

  Buffer = Group:GetVAttrBuffer(NGP_ATTR_VERTEX)

  VertexIndexStep = table.getn(Buffer)

  for i,v in ipairs(Buffer) do
   OBJFile:write(string.format("v %f %f %f\n",v[1],v[2],v[3]))
  end

  Buffer = Group:GetVAttrBuffer(NGP_ATTR_NORMAL)

  NormalIndexStep = table.getn(Buffer)

  for i,v in ipairs(Buffer) do
   OBJFile:write(string.format("vn %f %f %f\n",v[1],v[2],v[3]))
  end

  Buffer = Group:GetVAttrBuffer(NGP_ATTR_TEXCOORD0)

  TexCoordIndexStep = table.getn(Buffer)

  for i,v in ipairs(Buffer) do
   OBJFile:write(string.format("vt %f %f\n",v[1],v[2]))
  end

  Buffer = nil

  VertexIndexBuffer   = Group:GetVAttrIndexBuffer(NGP_ATTR_VERTEX,true,VertexIndexOffset)
  NormalIndexBuffer   = Group:GetVAttrIndexBuffer(NGP_ATTR_NORMAL,true,NormalIndexOffset)
  TexCoordIndexBuffer = Group:GetVAttrIndexBuffer(NGP_ATTR_TEXCOORD0,true,TexCoordIndexOffset)

  PrimitiveCount = table.getn(VertexIndexBuffer)

  for PrimitiveIndex = 1,PrimitiveCount do
   vi = VertexIndexBuffer[PrimitiveIndex]
   ni = NormalIndexBuffer[PrimitiveIndex]
   ti = TexCoordIndexBuffer[PrimitiveIndex]

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

function ExportMTLFile(MTLFileName)
 MTLFile = io.open(MTLFileName,"w")

 for GroupIndex,Group in VisibleGroupsIter(PlantModel) do
  Material = Group:GetMaterial()

  MTLFile:write(string.format("newmtl pmat%u\n",GroupIndex))
  MTLFile:write(string.format("Kd %f %f %f\n",Material.Color.R,Material.Color.G,Material.Color.B))

  if Material.TexNames[NGP_TEX_DIFFUSE] then
   MTLFile:write(string.format("map_Kd %s\n",GetTextureFileName(Material.TexNames[NGP_TEX_DIFFUSE])))
  end
 end

 MTLFile:close()
end

OBJFileName = ShowFileSaveDialog("Choose .OBJ file name")

if OBJFileName then
 MTLFileName = ShowFileSaveDialog("Choose .MTL file name")

 if MTLFileName then

  if     ExportPreferences.HiddenGroupsExportMode == NGP_ALWAYS then
   ExportHiddenGroups = true
  elseif ExportPreferences.HiddenGroupsExportMode == NGP_NEVER then
   ExportHiddenGroups = false
  else
   ExportHiddenGroups = ShowYesNoMessageBox('Export hidden branch groups?','Export mode')
  end

  if     ExportPreferences.OutVisRangeExportMode == NGP_ALWAYS then
   ExportOutVisRangeGroups = true
  elseif ExportPreferences.OutVisRangeExportMode == NGP_NEVER then
   ExportOutVisRangeGroups = false
  else
   ExportOutVisRangeGroups = ShowYesNoMessageBox('Export branch groups which are outside LOD visibility range?','Export mode')
  end

  ExportOBJFile(OBJFileName,MTLFileName)
  ExportMTLFile(MTLFileName)
 end
end

