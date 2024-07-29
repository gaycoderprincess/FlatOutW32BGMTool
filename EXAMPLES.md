# Examples and tips

## Blender FBX settings
When importing .fbx files from this tool, use these settings in Blender:

![import settings](https://i.imgur.com/Jd3zVaI.png)

When exporting them back, use these settings:

![export settings](https://i.imgur.com/Fh2Szm8.png)

## Exporting and importing a vehicle
- Enter `FlatOutW32BGMTool_gcp.exe path/to/body.bgm -export_fbx` into a commandline prompt
- Edit the .fbx and make your desired changes and additions
- Enter `FlatOutW32BGMTool_gcp.exe path/to/model.fbx -create_fouc_bgm` into a commandline prompt (Available formats are `-create_fo1_bgm`, `-create_fo2_bgm` and `-create_fouc_bgm`)
- Take the files ending in `_out.bgm` and `_out_crash.dat`, rename them and put them into the game files.

## Exporting and importing a track
- Enter `FlatOutW32BGMTool_gcp.exe path/to/track_geom.w32 -export_fbx` into a commandline prompt
- Edit the .fbx and make your desired changes and additions
- Enter `FlatOutW32BGMTool_gcp.exe path/to/track_geom.w32 path/to/model.fbx -import_all_surfaces -import_moved_props -import_cloned_props` into a commandline prompt
- Enter `FlatOutW32BGMTool_gcp.exe path/to/track_bvh.gen -empty_bvh_gen` into a commandline prompt
- If you need to remove grass or trees, enter `FlatOutW32BGMTool_gcp.exe -empty_plant_db` into a commandline prompt
- Copy the new .w32 and .gen files, rename them and put them into the game files.

## Converting a vehicle between games
- Enter `FlatOutW32BGMTool_gcp.exe path/to/body.bgm -export_fbx` into a commandline prompt
- Enter `FlatOutW32BGMTool_gcp.exe path/to/body_out.fbx -create_fouc_bgm` into a commandline prompt (Available formats are `-create_fo1_bgm`, `-create_fo2_bgm` and `-create_fouc_bgm`)
- Take and rename the appropriate files, and put them into your game files.

## Material help
Materials are given shaders decided based off of material names, here's a handy guide on how to name your materials (case-sensitive) to get the shader you want:

BGM shaders:
- Default without keywords: Car metal
- Car body/skin: `body` prefix
- Car diffuse: `interior` or `grille` prefix
- Car window: `window` prefix
- Car shear: `shear` prefix
- Car scale: `scale` prefix
- Car tire: `tire` prefix (same as car diffuse in FO1/FO2, separate shader in FOUC)
- Car rim: `rim` prefix
- Car lights: `light` prefix
- Shadow: `shadow` prefix
- Driver skin: `male` or `female` prefix

Additional notes:
- Materials with the `scaleshock` and `shearhock` prefix are configured to have no alpha

Map shaders:
- Default without keywords: Static prelit
- Alpha flag: `alpha_` or `wirefence_` prefix
- Double UV terrain: `dm_` or `terrain_` prefix
- Double UV terrain with specular: `sdm_` prefix
- Tree trunk: `treetrunk` prefix
- Tree branch: `alpha_treebranch` or `alpha_bushbranch` prefix
- Tree leaf: `alpha_treelod`, `alpha_treesprite`, `alpha_bushlod` or `alpha_bushsprite` prefix
- Water: Name your material `water` or give it the `puddle` prefix
- Dynamic objects: `_dynamic` suffix
- Dynamic objects with specular: `_dynamic_specular` suffix

## Advanced track creation example
- Take the Speedbowl track (arena3a) and export it using the tool.
- Create an empty track_bvh.gen using `-empty_bvh_gen` and optionally an empty plant_vdb.gen using `-empty_plant_db` as described above.
- Take your custom track model, and if it follows the exported .fbx's hierarchy (i.e. it was exported by the tool and you didn't add any non-conforming parts) then import it with:
`FlatOutW32BGMTool_gcp.exe path/to/speedbowl/track_geom.w32 path/to/model.fbx -import_and_match_all_surfaces`
- If it doesn't follow that hierarchy (i.e. it's a custom model you created or took from somewhere) then import it with:
`FlatOutW32BGMTool_gcp.exe path/to/speedbowl/track_geom.w32 path/to/model.fbx -import_and_match_all_meshes`
- If you do not wish to have dynamic props on the map, add `-remove_props` to the command.
- If you do wish to have props, first re-export the track with `FlatOutW32BGMTool_gcp.exe path/to/speedbowl/track_geom_out.w32 -export_fbx`
- Then move and copy-paste the dummies contained within CompactMesh as you like.
- Afterwards, re-import the track again using `FlatOutW32BGMTool_gcp.exe path/to/speedbowl/track_geom_out.w32 path/to/model.fbx -import_moved_props -ungroup_moved_props -import_cloned_props` (this will not affect the track's visual mesh, only the prop placements!)
- If you wish to have edited prop models as well, do your changes to the prop meshes in the existing hierarchy and then add `-import_surfaces` to the above command.
