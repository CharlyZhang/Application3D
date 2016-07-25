# UPDATE 

1. Refactor `CZObjModel`   
	- Redefine `ObjModel` as a simple model, which can only be created from `ModelFactory`.
	- Considering the compability to the former project, the `ObjModel` is abstrated, from which `CZObjModel` is derived. 
	- Convert`CZObjModel` to `Model`
	
2. Define `Render`
	- `Render` is repsonsible for rendering, and manages all graphic resources.
	
3. 	Define `SceneLoader`
	- `SceneLoader` is to load *scene file* from disk.

4. Refine `Application3D`
	- simplify interfaces.
	- add function to free graphic resources.
	
	

	