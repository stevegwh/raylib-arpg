import bpy

mats = bpy.data.materials

for mat in mats:
    (original, _, ext) = mat.name.rpartition(".")

    if ext.isnumeric() and mats.find(original) != -1:
        print("%s -> %s" % (mat.name, original))

        mat.user_remap(mats[original])
        mats.remove(mat)
