--[[

ngPlant-plugin : gmesh-generator
menu-name : Import .OBJ

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

NGP_ATTR_VERTEXN    = 1
NGP_ATTR_NORMALN    = 2
NGP_ATTR_TEXCOORD0N = 3
NGP_ATTR_TANGENTN   = 4
NGP_ATTR_BINORMALN  = 5

function filtered_lines(source_file)
 local iter = source_file:lines()

 return
  function()
   local l = iter()

   while l ~= nil do

    l = string.gsub(l,"^%s+","")
    l = string.gsub(l,"%s+$","")

    if string.len(l) > 0 and string.sub(l,1,1) ~= "#" then
     return l
    end

    l = iter()
   end
  end
end

function ivalues(t)
 local i = 0

 return function()
  i = i + 1
  return t[i]
 end
end

function faces_with_vert(vindices,vi,ni)
 local fi = 0
 local fn = table.getn(vindices)
 return function()
  while fi < fn do
   fi = fi + 1
   local f = vindices[fi]
   local v

   for v in ivalues(f) do
    if v[NGP_ATTR_VERTEXN] == vi and v[NGP_ATTR_NORMALN] == ni then
     return fi
    end
   end
  end
 end
end

function split(str)
 local r = {}
 local w

 for w in string.gfind(str,"[^%s]+") do
  table.insert(r,w)
 end

 return r
end

function split_ints(str)
 local r = {}
 local w

 for w in string.gfind(str,"[%d-]+") do
  table.insert(r,w)
 end

 return r
end

function add_attribute3(p,v)
 if table.getn(v) < 3 then
  error("invalid vertex attribute element count")
 end

 table.insert(p,{ tonumber(v[1]),tonumber(v[2]),tonumber(v[3]) })
end

function add_attribute2(p,v)
 if table.getn(v) < 2 then
  error("invalid vertex attribute element count")
 end

 table.insert(p,{ tonumber(v[1]),tonumber(v[2]) })
end

function add_face(f,i,vc,tc,nc)
 local c = table.getn(i)

 if c < 3 or c > 4 then
  error("face with unsupported vertex count (" .. c ..") found")
 end

 local p = {}
 local n

 for n = 1,c do
  local a = split_ints(i[n])

  if table.getn(a) ~= 3 then
   error("invalid attribute index count")
  end

  local vi,ti,ni = tonumber(a[1]),tonumber(a[2]),tonumber(a[3])

  if vi < 1 or vi > vc then
   error("invalid position attribute index")
  end

  if ti < 1 or ti > tc then
   error("invalid texture attribute index")
  end

  if ni < 1 or ni > nc then
   error("invalid normal attribute index")
  end

  table.insert(p, { vi, ni, ti, 1, 1 })
 end

 table.insert(f,p)
end

function itables_equal(t1,t2)
 if table.getn(t1) ~= table.getn(t2) then
  error("trying to compare arrays of different size")
 end

 local i

 for i = 1,table.getn(t1) do
  if t1[i] ~= t2[i] then
   return false
  end
 end

 return true
end

function find_attrs(a,v)
 local ai,av

 for ai,av in ipairs(a) do
  if itables_equal(av,v) then
   return ai
  end
 end

 return nil
end

function calc_tangent_spacef(vattrs,vindices)
 local ftangents  = {}
 local fbinormals = {}

 for f in ivalues(vindices) do
  local p0,p1,p2
  local q1 = {}
  local q2 = {}
  local s1,t1,s2,t2

  p0 = vattrs.vertex[f[1][NGP_ATTR_VERTEXN]]
  p1 = vattrs.vertex[f[2][NGP_ATTR_VERTEXN]]
  p2 = vattrs.vertex[f[3][NGP_ATTR_VERTEXN]]

  q1[1] = p1[1] - p0[1]
  q1[2] = p1[2] - p0[2]
  q1[3] = p1[3] - p0[3]
  q2[1] = p2[1] - p0[1]
  q2[2] = p2[2] - p0[2]
  q2[3] = p2[3] - p0[3]

  s1 = vattrs.texcoord0[f[2][NGP_ATTR_TEXCOORD0N]][1] -
        vattrs.texcoord0[f[1][NGP_ATTR_TEXCOORD0N]][1]
  t1 = vattrs.texcoord0[f[2][NGP_ATTR_TEXCOORD0N]][2] -
        vattrs.texcoord0[f[1][NGP_ATTR_TEXCOORD0N]][2]
  s2 = vattrs.texcoord0[f[3][NGP_ATTR_TEXCOORD0N]][1] -
        vattrs.texcoord0[f[1][NGP_ATTR_TEXCOORD0N]][1]
  t2 = vattrs.texcoord0[f[3][NGP_ATTR_TEXCOORD0N]][2] -
        vattrs.texcoord0[f[1][NGP_ATTR_TEXCOORD0N]][2]

  local d = s1 * t2 - s2 * t1

  if d > -0.00001 and d < 0.00001 then
   error("unable to generate tangent space basis (matrix is not invertible)")
  end

  d = 1.0 / d

  local t = {}
  local b = {}

  t[1] = d * (t2 * q1[1] - t1 * q2[1])
  t[2] = d * (t2 * q1[2] - t1 * q2[2])
  t[3] = d * (t2 * q1[3] - t1 * q2[3])

  b[1] = d * (-s2 * q1[1] + s1 * q2[1])
  b[2] = d * (-s2 * q1[2] + s1 * q2[2])
  b[3] = d * (-s2 * q1[3] + s1 * q2[3])

  table.insert(ftangents,t)
  table.insert(fbinormals,b)
 end

 return ftangents,fbinormals
end

function calc_tangent_spacev(vindices,vi,ni,ftangents,fbinormals)
 local fc = 0
 local tangent = {}
 local binormal = {}

 tangent[1]  = 0
 tangent[2]  = 0
 tangent[3]  = 0
 binormal[1] = 0
 binormal[2] = 0
 binormal[3] = 0

 for fi in faces_with_vert(vindices,vi,ni) do
  tangent[1]  = tangent[1] + ftangents[fi][1]
  tangent[2]  = tangent[2] + ftangents[fi][2]
  tangent[3]  = tangent[3] + ftangents[fi][3]
  binormal[1] = binormal[1] + fbinormals[fi][1]
  binormal[2] = binormal[2] + fbinormals[fi][2]
  binormal[3] = binormal[3] + fbinormals[fi][3]

  fc = fc + 1
 end

 if fc == 0 then
  error("no faces with vertex " .. vi .. "," .. ni .. " found")
 end

 tangent[1] = tangent[1] / fc
 tangent[2] = tangent[2] / fc
 tangent[3] = tangent[3] / fc
 binormal[1] = binormal[1] / fc
 binormal[2] = binormal[2] / fc
 binormal[3] = binormal[3] / fc

 return tangent,binormal
end

function mk_ortho(normal,tangent,binormal)
 local d1,d2
 local t = {}
 local b = {}

 d1 = normal[1] * tangent[1] + normal[2] * tangent[2] + normal[3] * tangent[3]

 t[1] = tangent[1] - d1 * normal[1]
 t[2] = tangent[2] - d1 * normal[2]
 t[3] = tangent[3] - d1 * normal[3]

 d1 = normal[1] * binormal[1] + normal[2] * binormal[2] + normal[3] * binormal[3]
 d2 = t[1] * binormal[1] + t[2] * binormal[2] + t[3] * binormal[3]

 b[1] = binormal[1] - d1 * normal[1] - d2 * t[1]
 b[2] = binormal[2] - d1 * normal[2] - d2 * t[2]
 b[3] = binormal[3] - d1 * normal[3] - d2 * t[3]

 return t,b
end

function normalize(v)
 local l = math.sqrt(v[1] * v[1] + v[2] * v[2] + v[3] * v[3])

 if l < 0.000005 then
  error("zero-length vector can not be normalized")
 end

 v[1] = v[1] / l
 v[2] = v[2] / l
 v[3] = v[3] / l

 return v
end

function fill_tangent_spacev(vattrs,vindices,ftangents,fbinormals)
 local f,v
 local t,b
 local attrs = {}

 for f in ivalues(vindices) do
  for v in ivalues(f) do
   local tsi = find_attrs(attrs,v[NGP_ATTR_VERTEXN],v[NGP_ATTR_NORMALN])

   if tsi == nil then
    t,b = calc_tangent_spacev(vindices,v[NGP_ATTR_VERTEXN],v[NGP_ATTR_NORMALN],ftangents,fbinormals)
    t,b = mk_ortho(vattrs.normal[v[NGP_ATTR_NORMALN]],t,b)

    t = normalize(t)
    b = normalize(b)

    table.insert(vattrs.tangent,t)
    table.insert(vattrs.binormal,b)

    tsi = table.getn(vattrs.tangent)
   end

   v[NGP_ATTR_TANGENTN]  = tsi
   v[NGP_ATTR_BINORMALN] = tsi
  end
 end
end

function convert2indexed(vattrs,vindices)
 local attrs =
  {
   vertex    = {},
   normal    = {},
   texcoord0 = {},
   binormal  = {},
   tangent   = {}
  }

 local indices = {}
 local iattrs = {}
 local f

 for f in ivalues(vindices) do
  -- get vattr-combination index for each vertex
  -- split quad into two triangles

  local vi = {}
  local v,i

  for v in ivalues(f) do
   local i = find_attrs(iattrs,v)

   if not i then
    table.insert(iattrs,v)
    table.insert(vi,table.getn(iattrs))
    table.insert(attrs.vertex,vattrs.vertex[v[NGP_ATTR_VERTEXN]])
    table.insert(attrs.normal,vattrs.normal[v[NGP_ATTR_NORMALN]])
    table.insert(attrs.texcoord0,vattrs.texcoord0[v[NGP_ATTR_TEXCOORD0N]])
    table.insert(attrs.tangent,vattrs.tangent[v[NGP_ATTR_TANGENTN]])
    table.insert(attrs.binormal,vattrs.binormal[v[NGP_ATTR_BINORMALN]])
   else
    table.insert(vi,i)
   end
  end

  if table.getn(vi) == 4 then
   table.insert(indices,{ vi[1], vi[2], vi[4] })
   table.insert(indices,{ vi[4], vi[2], vi[3] })
  else
   table.insert(indices,{ vi[1], vi[2], vi[3] })
  end
 end

 return attrs,indices
end

src_filename = ShowFileOpenDialog("Select .OBJ file to import...")

if src_filename == nil then
 return nil
end

source_file = io.open(src_filename,"r")

vattrs =
 {
  vertex    = {},
  normal    = {},
  texcoord0 = {},
  binormal  = {},
  tangent   = {}
 }

vindices = {}

for line in filtered_lines(source_file) do
 words = split(line)

 if table.getn(words) > 0 then
  k = words[1]
  table.remove(words,1)

  if     k == "v" then
   add_attribute3(vattrs.vertex,words)
  elseif k == "vt" then
   add_attribute2(vattrs.texcoord0,words)
  elseif k == "vn" then
   add_attribute3(vattrs.normal,words)
  elseif k == "f" then
   add_face(vindices,
            words,
            table.getn(vattrs.vertex),
            table.getn(vattrs.texcoord0),
            table.getn(vattrs.normal))
  elseif k == "o" or
         k == "mtllib" or
         k == "usemtl" or
         k == "usemap" or
         k == "g"      or
         k == "s" then
   -- skip unsignificant (for our target) lines
  else
   error("unsupported keyword (" .. k .. ") found in .obj file")
  end
 end
end

source_file:close()

ftangents,fbinormals = calc_tangent_spacef(vattrs,vindices)

fill_tangent_spacev(vattrs,vindices,ftangents,fbinormals)

attrs,indices = convert2indexed(vattrs,vindices)

return { vattrs = vattrs,
         vindices = vindices,
         attrs = attrs,
         indices = indices
       }

