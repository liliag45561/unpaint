﻿#include "pch.h"
#include "ModelsViewModel.h"
#include "ModelsViewModel.g.cpp"
#include "Infrastructure/DependencyContainer.h"
#include "Web/HuggingFaceClient.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Web;
using namespace std;
using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::System;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  const char* const ModelsViewModel::_modelFilter = "unpaint,stable_diffusion_model";

  ModelsViewModel::ModelsViewModel() :
    _availableModels(single_threaded_observable_vector<ModelViewModel>()),
    _installedModels(single_threaded_observable_vector<ModelViewModel>()),
    _modelRepository(dependencies.resolve<ModelRepository>())
  {
    UpdateAvailableModelsAsync();
    UpdateInstalledModels();
  }

  Windows::Foundation::Collections::IObservableVector<ModelViewModel> ModelsViewModel::AvailableModels()
  {
    return _availableModels;
  }

  bool ModelsViewModel::AreAvailableModelsEmpty()
  {
    return _availableModels.Size() == 0;
  }

  fire_and_forget ModelsViewModel::UpdateAvailableModelsAsync()
  {
    apartment_context callerContext;
    
    auto lifetime = get_strong();
    co_await resume_background();

    HuggingFaceClient client{};
    auto models = client.GetModels(_modelFilter);

    co_await callerContext;

    _availableModels.Clear();
    for (auto& model : models)
    {
      _availableModels.Append(CreateModelViewModel(model));
    }

    _propertyChanged(*this, PropertyChangedEventArgs(L"AreAvailableModelsEmpty"));
  }

  void ModelsViewModel::OpenAvailableModelWebsite()
  {
    auto uri = _availableModels.GetAt(_selectedAvailableModel).Uri;
    Launcher::LaunchUriAsync(Uri(uri));
  }

  int32_t ModelsViewModel::SelectedAvailableModel()
  {
    return _selectedAvailableModel;
  }

  void ModelsViewModel::SelectedAvailableModel(int32_t value)
  {
    _selectedAvailableModel = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsAvailableModelSelected"));
  }

  bool ModelsViewModel::IsAvailableModelSelected()
  {
    return _selectedAvailableModel != -1;
  }

  Windows::Foundation::Collections::IObservableVector<ModelViewModel> ModelsViewModel::InstalledModels()
  {
    return _installedModels;
  }

  bool ModelsViewModel::AreInstalledModelsEmpty()
  {
    return _installedModels.Size() == 0;
  }

  fire_and_forget ModelsViewModel::DownloadModelAsync()
  {
    auto modelId = _availableModels.GetAt(_selectedAvailableModel).Id;
    DownloadModelDialog dialog{ modelId };

    auto lifetime = get_strong();
    co_await dialog.ShowAsync();

    _modelRepository->Refresh();
  }

  void ModelsViewModel::OpenInstalledModelWebsite()
  {
    auto uri = _installedModels.GetAt(_selectedInstalledModel).Uri;
    Launcher::LaunchUriAsync(Uri(uri));
  }

  int32_t ModelsViewModel::SelectedInstalledModel()
  {
    return _selectedInstalledModel;
  }

  void ModelsViewModel::SelectedInstalledModel(int32_t value)
  {
    _selectedInstalledModel = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsInstalledModelSelected"));
  }

  bool ModelsViewModel::IsInstalledModelSelected()
  {
    return _selectedInstalledModel != -1;
  }

  event_token ModelsViewModel::PropertyChanged(PropertyChangedEventHandler const& value)
  {
    return _propertyChanged.add(value);
  }

  void ModelsViewModel::PropertyChanged(event_token const& token)
  {
    _propertyChanged.remove(token);
  }
  
  void ModelsViewModel::UpdateInstalledModels()
  {
    _installedModels.Clear();
    for (auto& model : _modelRepository->Models())
    {
      _installedModels.Append(CreateModelViewModel(model));
    }

    _propertyChanged(*this, PropertyChangedEventArgs(L"InstalledModels"));
  }
  
  ModelViewModel ModelsViewModel::CreateModelViewModel(const std::string& modelId)
  {
    return ModelViewModel{
      .Id = to_hstring(modelId),
      .Uri = to_hstring("https://huggingface.co/" + modelId)
    };
  }
}
