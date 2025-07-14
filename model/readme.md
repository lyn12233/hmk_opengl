# structures and types

### aiScene

- root_node
- num_meshes/**meshes
- num_materials/**materials
- num_textures/**textures
- name:str

### aiNode

- *parent
- num_children/**children
- num_meshes/\*meshes:\*int
- transformation: mat4x4
- name: str

### aiMesh

- num_vertices/*vertices, *normals, *tangents, *bitangents, *colors\[8], *coords\[8]
- num_faces/*faces
- material_idx: int
- name: str

### aiFace

- num_indices/*indices

## types

### aiTexture

- width,height/\*pcData: *aiTexel
- filename: str

### aiMaterial
?
