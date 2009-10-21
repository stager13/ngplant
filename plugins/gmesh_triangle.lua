--[[

ngPlant-plugin : gmesh-generator
menu-name : Triangle

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

function CreateTriangle(height,width,attach_mode)
  if attach_mode == "edge" then
    vertex_attrs = { { -width * 0.5, 0.0, 0.0 },
                     {  width * 0.5, 0.0, 0.0 },
                     { 0.0, height, 0.0 } }

    texcoord0_attrs = { { 0.0, 0.0 }, { 1.0, 0.0 }, { 0.5, 1.0 } }
  else
    vertex_attrs = { {  0.0, 0.0, 0.0 },
                     {  width * 0.5, height, 0.0 },
                     { -width * 0.5, height, 0.0 } }

    texcoord0_attrs = { { 0.5, 0.0 }, { 1.0, 1.0 }, { 0.0, 1.0 } }
  end

  normal_attr = {  0.0, 0.0, 1.0  }
  binormal_attr = { 0.0, 1.0, 0.0 }
  tangent_attr = { 1.0, 0.0, 0.0 }

  Triangle = {
           vattrs =
            {
             vertex = vertex_attrs,
             normal = { normal_attr },
             texcoord0 = texcoord0_attrs,
             binormal = { binormal_attr },
             tangent = { tangent_attr }
            },

           vindices = { {
                         { 1, 1, 1, 1, 1},
                         { 2, 1, 2, 1, 1},
                         { 3, 1, 3, 1, 1},
                        } },

           attrs =
            {
             vertex = vertex_attrs,

             normal =  { normal_attr ,
                         normal_attr ,
                         normal_attr },

             texcoord0 = texcoord0_attrs,

             binormal =  { binormal_attr,
                           binormal_attr,
                           binormal_attr },

             tangent =  { tangent_attr,
                          tangent_attr,
                          tangent_attr }
            },

           indices = { { 1, 2, 3 } }
          }

  return Triangle
 end

Params = ShowParameterDialog(
 {
  {
   label   = "Height",
   name    = "Height",
   type    = "number",
   default = 0.5
  },
  {
   label   = "Width",
   name    = "Width",
   type    = "number",
   default = 0.5
  },
  {
   label   = "Attached by",
   name    = "AttachMode",
   type    = "choice",
   choices = { "edge", "vertex" }
  }
 })

if Params == nil then
 return nil
else
 return CreateTriangle(Params.Height,Params.Width,Params.AttachMode)
end

