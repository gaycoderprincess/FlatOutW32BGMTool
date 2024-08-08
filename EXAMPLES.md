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
- Enter `FlatOutW32BGMTool_gcp.exe path/to/track_geom.w32 -export_fbx -skip_hidden_props_a` into a commandline prompt (for track variant B or C use `-skip_hidden_props_b` or `-skip_hidden_props_c`)
- Edit the .fbx and make your desired changes and additions
- Enter `FlatOutW32BGMTool_gcp.exe path/to/model.fbx -create_fouc_w32` into a commandline prompt (Available formats are `-create_fo1_w32`, `-create_fo2_w32` and `-create_fouc_w32`)
- If you need to remove grass or other vegetation, enter `FlatOutW32BGMTool_gcp.exe -empty_plant_db` into a commandline prompt
- If you experience culling issues, delete `track_spvs.gen` from your track geometry folder.
- Copy the new .w32 and .gen files, rename them and put them into the game files.

## Exporting and importing a track while keeping trees intact in FOUC
- Enter `FlatOutW32BGMTool_gcp.exe path/to/track_geom.w32 -export_fbx -skip_hidden_props_a` into a commandline prompt (for track variant B or C use `-skip_hidden_props_b` or `-skip_hidden_props_c`)
- Edit the .fbx and make your desired changes and additions
- Enter `FlatOutW32BGMTool_gcp.exe path/to/track_geom.w32 path/to/model.fbx -import_all_surfaces -import_moved_props -import_cloned_props` into a commandline prompt
- If you need to remove grass or other vegetation, enter `FlatOutW32BGMTool_gcp.exe -empty_plant_db` into a commandline prompt
- If you experience culling issues, delete `track_spvs.gen` from your track geometry folder.
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
- Materials with the `_alpha` suffix are forced to have alpha regardless of the prefix

Map shaders:
- Default without keywords: Static prelit
- Alpha flag: `alpha_` or `wirefence_` prefix or `_alpha` suffix
- Double UV terrain: `dm_`, `terrain_`, `road_tarmac` or `road_gravel` prefix
- Double UV terrain with specular: `sdm_` prefix
- Tree trunk: `treetrunk` prefix
- Tree branch: `alpha_treebranch` or `alpha_bushbranch` prefix
- Tree leaf: `alpha_treelod`, `alpha_treesprite`, `alpha_bushlod` or `alpha_bushsprite` prefix
- Water: Name your material `water` or give it the `puddle` prefix
- Static window glass: `static_windows` prefix
- Dynamic window glass: `dynamic_windows` prefix
- Dynamic objects: `_dynamic` suffix
- Dynamic objects with specular: `_dynamic_specular` suffix
