import os
import filecmp
import re
from collections import defaultdict
import argparse

def normalize_obj_content(file_path):
    """Read OBJ file and return content with normalized mtllib and object name lines."""
    with open(file_path, 'r') as f:
        lines = f.readlines()
    
    normalized_lines = []
    for line in lines:
        # Skip mtllib and object name lines
        if line.startswith('mtllib') or line.startswith('o '):
            continue
        normalized_lines.append(line)
    
    return ''.join(normalized_lines)

def find_duplicate_objs(mesh_folder):
    """Find groups of duplicate OBJ files based on their content."""
    obj_files = [f for f in os.listdir(mesh_folder) if f.endswith('.obj')]
    
    # Group files by their content
    content_groups = defaultdict(list)
    for obj_file in obj_files:
        file_path = os.path.join(mesh_folder, obj_file)
        content = normalize_obj_content(file_path)
        content_groups[content].append(obj_file)
    
    # Return only groups with duplicates
    return {content: files for content, files in content_groups.items() if len(files) > 1}

def update_transform_files(output_folder, obj_mapping):
    """Update transform files to reference the canonical OBJ file."""
    transform_files = [f for f in os.listdir(output_folder) if f.endswith('.txt')]
    
    for transform_file in transform_files:
        file_path = os.path.join(output_folder, transform_file)
        updated_content = []
        modified = False
        
        with open(file_path, 'r') as f:
            lines = f.readlines()
            
        for line in lines:
            if line.startswith('mesh:'):
                mesh_name = line.split(':')[1].strip()
                obj_name = mesh_name.split('.obj')[0] + '.obj'
                
                if obj_name in obj_mapping:
                    new_obj = obj_mapping[obj_name]
                    updated_line = f'mesh: {new_obj}\n'
                    if updated_line != line:
                        modified = True
                        line = updated_line
            
            updated_content.append(line)
        
        if modified:
            with open(file_path, 'w') as f:
                f.writelines(updated_content)
            print(f"Updated references in {transform_file}")

def clean_mtl_files(mesh_folder, removed_objs):
    """Remove MTL files associated with removed OBJ files."""
    for obj_file in removed_objs:
        mtl_file = obj_file.replace('.obj', '.mtl')
        mtl_path = os.path.join(mesh_folder, mtl_file)
        if os.path.exists(mtl_path):
            os.remove(mtl_path)
            print(f"Removed associated MTL file: {mtl_file}")

def deduplicate_objs(base_folder):
    """Main function to deduplicate OBJ files and update references."""
    mesh_folder = os.path.join(base_folder, "mesh")
    if not os.path.exists(mesh_folder):
        print(f"Mesh folder not found: {mesh_folder}")
        return
    
    print("Finding duplicate OBJ files...")
    duplicate_groups = find_duplicate_objs(mesh_folder)
    
    if not duplicate_groups:
        print("No duplicate OBJ files found.")
        return
    
    # Create mapping from duplicate files to their canonical version
    obj_mapping = {}
    removed_objs = set()
    
    for files in duplicate_groups.values():
        # Use the first file as the canonical version
        canonical = files[0]
        print(f"\nKeeping {canonical} as canonical version")
        
        # Map all other files to the canonical version and remove them
        for duplicate in files[1:]:
            obj_mapping[duplicate] = canonical
            duplicate_path = os.path.join(mesh_folder, duplicate)
            os.remove(duplicate_path)
            removed_objs.add(duplicate)
            print(f"Removed duplicate: {duplicate}")
    
    # Update transform files to reference the canonical OBJ files
    print("\nUpdating transform file references...")
    update_transform_files(base_folder, obj_mapping)
    
    # Clean up associated MTL files
    print("\nCleaning up associated MTL files...")
    clean_mtl_files(mesh_folder, removed_objs)
    
    print("\nDeduplication complete!")

def main():
    parser = argparse.ArgumentParser(description='Deduplicate OBJ files and update their references.')
    parser.add_argument('path', help='Path to the dungeon-map directory')
    args = parser.parse_args()
    
    # Verify the path exists
    if not os.path.exists(args.path):
        print(f"Error: Path does not exist: {args.path}")
        return
    
    deduplicate_objs(args.path)

if __name__ == "__main__":
    main()