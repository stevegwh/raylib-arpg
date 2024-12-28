import bpy
import os
import shutil

print(f'=======================')

basedir = os.path.dirname(bpy.data.filepath)
if not basedir:
    raise Exception("Blend file is not saved")

output_folder = os.path.join(basedir, "dungeon-map")
if os.path.exists(output_folder):
    for filename in os.listdir(output_folder):
        file_path = os.path.join(output_folder, filename)
        try:
            if os.path.isfile(file_path) or os.path.islink(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)
        except Exception as e:
            print(f'Failed to delete {file_path}. Reason: {e}')
else:
    os.makedirs(output_folder)

mesh_folder = os.path.join(output_folder, "mesh")
os.makedirs(mesh_folder, exist_ok=True)

bpy.ops.file.pack_all()

view_layer = bpy.context.view_layer
obj_active = view_layer.objects.active
selection = bpy.context.selected_objects
bpy.ops.object.select_all(action='DESELECT')

unique_meshes = {}


def handle_spawner(obj, output_folder):
    print(f"\nDEBUG INFO for object: {obj.name}")

    # Check if we have mesh data and try to get the property
    spawner_name = "NOT_FOUND"
    if "spawner_name" in obj:
        try:
            spawner_name = obj["spawner_name"]
            print(f"Found spawner_name in mesh data: {spawner_name}")
        except:
            print("Failed to access spawner_name")
    else:
        print("No spawner_name property found")

    transform_file = os.path.join(output_folder, bpy.path.clean_name(obj.name) + ".txt")
    with open(transform_file, 'w') as f:
        f.write(f"type: spawner\n")
        f.write(f"name: {obj.name}\n")
        f.write(f"location: {obj.location.x:.6f} {obj.location.z:.6f} {-obj.location.y:.6f}\n")
        f.write(f"rotation: {obj.rotation_euler.x:.6f} {obj.rotation_euler.z:.6f} {obj.rotation_euler.y:.6f}\n")

        if "Enemy" in obj.name:
            spawner_type = "ENEMY"
        elif "Player" in obj.name:
            spawner_type = "PLAYER"
        elif "NPC" in obj.name:
            spawner_type = "NPC"
        else:
            spawner_type = "UNKNOWN"

        f.write(f"spawner_type: {spawner_type}\n")

        # Handle spawned_name based on type
        if spawner_type == "PLAYER":
            f.write("spawner_name: PLAYER\n")
        else:
            f.write(f"spawner_name: {spawner_name}\n")

    print(f"Exported spawner: {transform_file}")


def handle_light(obj, output_folder):
    if obj.data.type == 'POINT':
        light_file = os.path.join(output_folder, bpy.path.clean_name(obj.name) + ".txt")
        with open(light_file, 'w') as f:
            r = obj.data.color.r * 255
            g = obj.data.color.g * 255
            b = obj.data.color.b * 255
            f.write(f"type: light\n")
            f.write(f"light_type: point\n")
            f.write(f"name: {obj.name}\n")
            f.write(f"location: {obj.location.x:.6f} {obj.location.z:.6f} {-obj.location.y:.6f}\n")
            f.write(f"color: {int(r)} {int(g)} {int(b)}\n")
            f.write(f"strength: {int(obj.data.energy)}\n")

        print(f"Exported light: {light_file}")
    elif obj.data.type == 'SUN':
        light_file = os.path.join(output_folder, bpy.path.clean_name(obj.name) + ".txt")
        with open(light_file, 'w') as f:
            r = obj.data.color.r * 255
            g = obj.data.color.g * 255
            b = obj.data.color.b * 255
            f.write(f"type: light\n")
            f.write(f"light_type: sun\n")
            f.write(f"name: {obj.name}\n")
            f.write(f"location: {obj.location.x:.6f} {obj.location.z:.6f} {-obj.location.y:.6f}\n")
            f.write(f"color: {int(r)} {int(g)} {int(b)}\n")
            f.write(f"strength: {int(obj.data.energy)}\n")

        print(f"Exported light: {light_file}")


def copy_textures_to_mesh():
    textures_dir = os.path.join(basedir, "textures")
    if os.path.exists(textures_dir):
        for filename in os.listdir(textures_dir):
            src = os.path.join(textures_dir, filename)
            dst = os.path.join(mesh_folder, filename)
            shutil.copy2(src, dst)
        print("Copied textures to mesh directory")


bpy.ops.file.unpack_all(method='WRITE_LOCAL')
copy_textures_to_mesh()

for obj in selection:
    obj.select_set(True)

    if obj.type == 'NONE' or obj is None or obj.data is None:
        continue

    if "Spawner" in obj.name:
        handle_spawner(obj, output_folder)
    elif obj.type == 'LIGHT':
        handle_light(obj, output_folder)
    else:
        if obj.data.name not in unique_meshes:
            original_transform = obj.matrix_world.copy()
            obj.matrix_world.identity()

            view_layer.objects.active = obj
            mesh_name = bpy.path.clean_name(obj.data.name)
            fn = os.path.join(mesh_folder, mesh_name)

            bpy.ops.wm.obj_export(filepath=fn + ".obj", export_selected_objects=True, path_mode='STRIP')

            obj.matrix_world = original_transform

            unique_meshes[obj.data.name] = mesh_name
            print("Exported mesh:", fn + ".obj")

        location = obj.location
        rotation = obj.rotation_euler
        scale = obj.scale

        transform_file = os.path.join(output_folder, bpy.path.clean_name(obj.name) + ".txt")
        with open(transform_file, 'w') as f:
            f.write(f"type: mesh\n")
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

bpy.ops.file.pack_all()

# Clean up textures folder after packing
textures_dir = os.path.join(basedir, "textures")
if os.path.exists(textures_dir):
    shutil.rmtree(textures_dir)
    print("Cleaned up textures directory")

print("Export completed.")
