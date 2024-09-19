import bpy
import os

# Export to 'output' folder relative to blend file location
basedir = os.path.dirname(bpy.data.filepath)
if not basedir:
    raise Exception("Blend file is not saved")

# Create 'output' folder if it doesn't exist
output_folder = os.path.join(basedir, "output")
os.makedirs(output_folder, exist_ok=True)

# Create 'output/meshes' folder if it doesn't exist
mesh_folder = os.path.join(output_folder, "mesh")
os.makedirs(mesh_folder, exist_ok=True)

view_layer = bpy.context.view_layer
obj_active = view_layer.objects.active
selection = bpy.context.selected_objects
bpy.ops.object.select_all(action='DESELECT')

# Dictionary to store unique meshes
unique_meshes = {}

for obj in selection:
    obj.select_set(True)
    
    # Check if the mesh is already exported
    if obj.data.name not in unique_meshes:
        # Set transform to 0,0,0
        original_transform = obj.matrix_world.copy()
        obj.matrix_world.identity()
        
        # Some exporters only use the active object
        view_layer.objects.active = obj
        mesh_name = bpy.path.clean_name(obj.data.name)
        fn = os.path.join(mesh_folder, mesh_name)
        
        # Export OBJ files
        bpy.ops.wm.obj_export(filepath=fn + ".obj", export_selected_objects=True, path_mode='STRIP')
        
        # Restore original transform
        obj.matrix_world = original_transform
        
        unique_meshes[obj.data.name] = mesh_name
        print("Exported mesh:", fn + ".obj")
    
    # Prepare transform data
    location = obj.location
    rotation = obj.rotation_euler
    scale = obj.scale
    
    # Export transform data to text file (Converted to "Unity" coords)
    transform_file = os.path.join(output_folder, bpy.path.clean_name(obj.name) + ".txt")
    with open(transform_file, 'w') as f:
        f.write(f"name: {obj.name}\n")
        f.write(f"mesh: {unique_meshes[obj.data.name]}.obj\n")
        f.write(f"location: {location.x:.6f} {location.z:.6f} {-location.y:.6f}\n")
        f.write(f"rotation: {rotation.x:.6f} {rotation.z:.6f} {rotation.y:.6f}\n")
        f.write(f"scale: {scale.x:.6f} {scale.z:.6f} {scale.y:.6f}\n")
    
    print("Exported transform:", transform_file)
    
    obj.select_set(False)

view_layer.objects.active = obj_active
for obj in selection:
    obj.select_set(True)

print("Export completed.")