#include "Geode/DefaultInclude.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/modify/Modify.hpp"
#include <BackupsLayer.h>
#include <Geode/loader/Setting.hpp>
#include <Geode/loader/SettingEvent.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include <Geode/utils/general.hpp>
#include <include.h>
#include <string>

struct MyMenuLayer : Modify<MyMenuLayer, MenuLayer> {
  bool init() {
    if (!MenuLayer::init())
      return false;

    if (!Mod::get()->getSettingValue<bool>("Button_In_Options_Menu")) {
      auto sprite = CCSprite::createWithSpriteFrameName("GJ_duplicateBtn_001.png");

      auto button = CCMenuItemSpriteExtra::create(sprite, nullptr, this, menu_selector(MyMenuLayer::OpenBackupsLayerButton));
      button->setZOrder(-1);

      auto menu = static_cast<CCMenu*>(this->getChildByID("right-side-menu"));
      menu->setPositionY(menu->getPositionY() - sprite->getContentSize().height / 2);

      menu->addChild(button);
      menu->updateLayout();
    }

    return true;
  }

  void OpenBackupsLayerButton(CCObject* target) { BackupsLayer::create()->show(this); }
};

struct MyOptionsLayer : Modify<MyOptionsLayer, OptionsLayer> {
  void customSetup() {
    OptionsLayer::customSetup();

    if (Mod::get()->getSettingValue<bool>("Button_In_Options_Menu")) {
      auto sprite = CCSprite::createWithSpriteFrameName("GJ_duplicateBtn_001.png");

      auto button = CCMenuItemSpriteExtra::create(sprite, nullptr, this, menu_selector(MyOptionsLayer::OpenBackupsLayerButton));

      auto menu = CCMenu::create();

      menu->addChild(button);
      this->m_listLayer->getParent()->addChild(menu);

      auto winSize = CCDirector::sharedDirector()->getWinSize();
      menu->setPosition({(winSize.width / (569 / 535.5f)), (winSize.height / (320 / 30))});
    }
  }

  void OpenBackupsLayerButton(CCObject* target) { BackupsLayer::create()->show(this); }
};

struct MyGameManager : Modify<MyGameManager, GameManager> {
  void doQuickSave() {
    GameManager::doQuickSave();
    if (Mod::get()->getSettingValue<bool>("Auto_Backup")) {
      geode::Result<> res = geode::utils::file::createDirectory(Mod::get()->getSaveDir() / "Backups");
      res = geode::utils::file::createDirectory(Mod::get()->getSaveDir() / "Backups/Auto-Backups");

      ghc::filesystem::path pathToAutosaves = Mod::get()->getSaveDir() / "Backups/Auto-Backups";

      auto readBackups = file::readDirectory(pathToAutosaves);

      int maxAmount = Mod::get()->getSettingValue<int64_t>("Max_Auto_Backups");

      typedef struct {
        ghc::filesystem::path path;
        std::chrono::milliseconds dateOfCreation;
      } backupWPath;

      std::vector<backupWPath> pathsWithTime;

      if (readBackups.value().size() == maxAmount) {
        for (int i = 0; i < readBackups.value().size(); i++) {
          auto Date = ghc::filesystem::last_write_time(readBackups.value()[i]);
          auto DateInMili = std::chrono::duration_cast<std::chrono::milliseconds>(Date.time_since_epoch());

          backupWPath meRN;
          meRN.dateOfCreation = DateInMili;
          meRN.path = readBackups.value()[i];
          pathsWithTime.push_back(meRN);

          int currentMEIndex = pathsWithTime.size() - 1;

          backupWPath prevSave;

          for (int b = 0; b < pathsWithTime.size(); b++) {
            if (currentMEIndex != 0) {
              if (pathsWithTime[currentMEIndex - 1].dateOfCreation > meRN.dateOfCreation) {
                prevSave = pathsWithTime[currentMEIndex - 1];
                pathsWithTime[currentMEIndex - 1] = meRN;
                pathsWithTime[currentMEIndex] = prevSave;
              }
            }
          }
        }

        ghc::filesystem::remove_all(pathsWithTime[0].path);
      }

      auto creationTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      const char* Time = std::ctime(&creationTime);
      std::string t = Time;
      std::string fileName = "Backups/Auto-Backups/--AutoBackup-- " + t;
      for (size_t i = 0; i < fileName.length(); i++) {
        if (fileName[i] == ':') {
          fileName[i] = '-';
        }
        if (fileName[i] == ' ') {
          fileName[i] = '_';
        }
      }
      fileName.erase(std::remove(fileName.begin(), fileName.end(), '\n'), fileName.cend());
      ghc::filesystem::path fullpath = Mod::get()->getSaveDir() / fileName;
      res = file::createDirectory(fullpath);

      std::string tempWPath = CCFileUtils::get()->getWritablePath();
      ghc::filesystem::path GDAPPDATAPATH(tempWPath);

      ghc::filesystem::copy(GDAPPDATAPATH / "CCGameManager.dat", fullpath);
      ghc::filesystem::copy(GDAPPDATAPATH / "CCGameManager2.dat", fullpath);
      ghc::filesystem::copy(GDAPPDATAPATH / "CCLocalLevels.dat", fullpath);
      ghc::filesystem::copy(GDAPPDATAPATH / "CCLocalLevels2.dat", fullpath);

      ghc::filesystem::path dataPath = fullpath / "Backup.dat";
      std::ofstream bkData(dataPath);

      bkData << "Auto-Backup" << std::endl;
      std::string timeDisp = "-" + t;
      bkData << timeDisp << std::endl;

      bkData.close();
    }
  }
};

struct Util {
  static void restart_game() { game::restart(); }
};

$execute {
  if (Mod::get()->getSettingValue<bool>("Auto_Backup")) {
    geode::Result<> res = geode::utils::file::createDirectory(Mod::get()->getSaveDir() / "Backups");
    res = geode::utils::file::createDirectory(Mod::get()->getSaveDir() / "Backups/Auto-Backups");

    ghc::filesystem::path pathToAutosaves = Mod::get()->getSaveDir() / "Backups/Auto-Backups";

    auto readBackups = file::readDirectory(pathToAutosaves);

    int maxAmount = Mod::get()->getSettingValue<int64_t>("Max_Auto_Backups");

    typedef struct {
      ghc::filesystem::path path;
      std::chrono::milliseconds dateOfCreation;
    } backupWPath;

    std::vector<backupWPath> pathsWithTime;

    if (readBackups.value().size() == maxAmount) {
      for (int i = 0; i < readBackups.value().size(); i++) {
        auto Date = ghc::filesystem::last_write_time(readBackups.value()[i]);
        auto DateInMili = std::chrono::duration_cast<std::chrono::milliseconds>(Date.time_since_epoch());

        backupWPath meRN;
        meRN.dateOfCreation = DateInMili;
        meRN.path = readBackups.value()[i];
        pathsWithTime.push_back(meRN);

        int currentMEIndex = pathsWithTime.size() - 1;

        backupWPath prevSave;

        for (int b = 0; b < pathsWithTime.size(); b++) {
          if (currentMEIndex != 0) {
            if (pathsWithTime[currentMEIndex - 1].dateOfCreation > meRN.dateOfCreation) {
              prevSave = pathsWithTime[currentMEIndex - 1];
              pathsWithTime[currentMEIndex - 1] = meRN;
              pathsWithTime[currentMEIndex] = prevSave;
            }
          }
        }
      }

      ghc::filesystem::remove_all(pathsWithTime[0].path);
    }

    auto creationTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const char* Time = std::ctime(&creationTime);
    std::string t = Time;
    std::string fileName = "Backups/Auto-Backups/--AutoBackup-- " + t;
    for (size_t i = 0; i < fileName.length(); i++) {
      if (fileName[i] == ':') {
        fileName[i] = '-';
      }
      if (fileName[i] == ' ') {
        fileName[i] = '_';
      }
    }
    fileName.erase(std::remove(fileName.begin(), fileName.end(), '\n'), fileName.cend());
    ghc::filesystem::path fullpath = Mod::get()->getSaveDir() / fileName;
    res = file::createDirectory(fullpath);

    std::string tempWPath = CCFileUtils::get()->getWritablePath();
    ghc::filesystem::path GDAPPDATAPATH(tempWPath);

    ghc::filesystem::copy(GDAPPDATAPATH / "CCGameManager.dat", fullpath);
    ghc::filesystem::copy(GDAPPDATAPATH / "CCGameManager2.dat", fullpath);
    ghc::filesystem::copy(GDAPPDATAPATH / "CCLocalLevels.dat", fullpath);
    ghc::filesystem::copy(GDAPPDATAPATH / "CCLocalLevels2.dat", fullpath);

    ghc::filesystem::path dataPath = fullpath / "Backup.dat";
    std::ofstream bkData(dataPath);

    bkData << "Auto-Backup" << std::endl;
    std::string timeDisp = "-" + t;
    bkData << timeDisp << std::endl;

    bkData.close();
  }

  geode::listenForSettingChanges("Backup_Save_Path", +[](std::string value) { Util::restart_game(); });
}
