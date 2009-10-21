--[[

ngPlant-plugin : gmesh-generator
menu-name : Rhomb

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

function CreateRhomb(height,width)
  Rhomb = {
           vattrs =
            {
             vertex = { { 0.0, 0.0, 0.0 },
                        { width * 0.5, height * 0.5, 0.0 },
                        { 0.0, height, 0.0 },
                        { -width * 0.5, height * 0.5, 0.0 } },
             normal = { { 0.0, 0.0, 1.0 } },
             texcoord0 = { { 0.5, 0.0 },
                           { 1.0, 0.5 },
                           { 0.5, 1.0 },
                           { 0.0, 0.5 } },
             binormal = { { 0.0, 1.0, 0.0 } },
             tangent = { { 1.0, 0.0, 0.0 } }
            },

           vindices = { {
                         { 1, 1, 1, 1, 1},
                         { 2, 1, 2, 1, 1},
                         { 3, 1, 3, 1, 1},
                         { 4, 1, 4, 1, 1}
                        } },

           attrs =
            {
             vertex =  { { 0.0, 0.0, 0.0 },
                         { width * 0.5, height * 0.5, 0.0 },
                         { -width * 0.5, height * 0.5, 0.0 },
                         { 0.0, height, 0.0 } },

             normal =  { { 0.0, 0.0, 1.0 },
                         { 0.0, 0.0, 1.0 },
                         { 0.0, 0.0, 1.0 },
                         { 0.0, 0.0, 1.0 } },

             texcoord0 = { { 0.5, 0.0 },
                           { 1.0, 0.5 },
                           { 0.0, 0.5 },
                           { 0.5, 1.0 } },

             binormal =  { { 0.0, 1.0, 0.0 },
                           { 0.0, 1.0, 0.0 },
                           { 0.0, 1.0, 0.0 },
                           { 0.0, 1.0, 0.0 } },

             tangent =  { { 1.0, 0.0, 0.0 },
                          { 1.0, 0.0, 0.0 },
                          { 1.0, 0.0, 0.0 },
                          { 1.0, 0.0, 0.0 } }
            },

           indices = { { 1, 2, 3 }, { 3, 2, 4} }
          }

  return Rhomb
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
  }
 })

if Params == nil then
 return nil
else
 return CreateRhomb(Params.Height,Params.Width)
end

