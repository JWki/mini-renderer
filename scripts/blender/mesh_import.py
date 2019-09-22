import os 

def import_mesh(input, output):

    import bpy
    import MeshExport


    # Clear existing objects.
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=input, axis_forward='-Z', axis_up='Y', directory = "", filter_glob="*.fbx", ui_tab='MAIN', use_manual_orientation=False, global_scale=1, bake_space_transform=False, use_custom_normals=True, use_image_search=True, use_alpha_decals=False, decal_offset=0, use_anim=True, anim_offset=1, use_custom_props=True, use_custom_props_enum_as_string=True, ignore_leaf_bones=False, force_connect_children=False, automatic_bone_orientation=False, primary_bone_axis='Y', secondary_bone_axis='X', use_prepost_rot=True)
    #MeshExport.save(bpy.context, output, use_selection=False, global_matrix=mathutils.Matrix())
    MeshExport.register()
    bpy.ops.export_scene.gtmesh(filepath=output)

def main():
    import sys 
    import argparse

    argv = sys.argv 
    if "--" not in argv:
        argv = []
    else:
        argv = argv[argv.index("--") + 1:]

    parser = argparse.ArgumentParser(description="Help")

    parser.add_argument("-i", "--input", dest="input", type=str, required=True, help="Source file, must be .fbx")
    parser.add_argument("-o", "--output", dest="output", type=str, required=True, help="Output path")


    args = parser.parse_args(argv)
    if not argv:
        parser.print_help()
        return
    
    import errno

    inputPath = os.path.join(os.getcwd(), args.input)
    outputPath = os.path.join(os.getcwd(), args.output)
    if not os.path.exists(os.path.dirname(outputPath)):
        try:
            os.makedirs(os.path.dirname(outputPath))
        except OSError as exc: 
            if exc.errno != errno.EEXIST:
                raise

    import_mesh(inputPath, outputPath)

    bpy.ops.wm.read_factory_settings(use_empty=True) # avoid memory leakage
    print("batch job finished, existing")


if __name__ == "__main__":
    main()