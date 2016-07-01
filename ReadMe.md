# Application3D

This project is derived from [**objViewer**](https://github.com/CharlyZhang/objViewer). It extends the function as below:

- Shape Aniamtions (developing)

## Structure
- **data** : obj models
- **src** : source files writen in c++ / oc
- **resource** : glsl files, config files
- **ios** : workplace on iOS
	- **lib** : project compiling Application3D, including a target to create static lib (**libApplication3D**)
	- **vendor** : the 3rd-party lib
	- *3DViewer.xcodeproj* : project using **libApplication3D**
	- **Application3DDemo** : similar as *3DViewer.xcodeproj*, used for developping lib
	- *Application3D.xcworkspace* : workplace for developping lib
	
