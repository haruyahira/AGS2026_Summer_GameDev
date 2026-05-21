#pragma once
#include<memory>
#include<list>
class Scene;
class Input;
class SceneController
{
private:
	std::list<std::shared_ptr<Scene>> scenes_;
public:

	void ChangeScene(std::shared_ptr<Scene> scene);
	void PushScene(std::shared_ptr<Scene>scene);
	void PopScene();
	void ResetScene(std::shared_ptr<Scene>scene);
	void Update(Input& input);
	void Draw();
};

