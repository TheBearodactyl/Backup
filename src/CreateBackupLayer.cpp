#include "CreateBackupLayer.h"
#include <BackupCell.h>
#include <Geode/ui/TextInput.hpp>
#include <string>

CreateBackupLayer* CreateBackupLayer::create(BackupsLayer* const& _parentLayer) {
  auto ret = new CreateBackupLayer();
  if (ret && ret->init(280, 99, _parentLayer, "GJ_square01.png", {0.f, 0.f, 80.f, 80.f})) {
    ret->autorelease();
  } else {
    delete ret;
    ret = nullptr;
  }
  return ret;
}

bool CreateBackupLayer::setup(BackupsLayer* const& _parentLayer) {
  // create window
  auto winSize = CCDirector::sharedDirector()->getWinSize();

  parentLayer = _parentLayer;
  if (!this->initWithColor({0, 0, 0, 105}))
    return false;
  m_mainLayer = CCLayer::create();
  this->addChild(m_mainLayer);

  CCLabelBMFont* Label = CCLabelBMFont::create("Create Backup", "bigFont.fnt");
  Label->setPosition(parentLayer->GetResFixedScale({285, 194}));
  Label->setScale(0.75f);
  m_mainLayer->addChild(Label);

  NameInput = TextInput::create(220, "Backup Name");
  // NameInput = InputNode::create(220, "Backup Name", "bigFont.fnt", "", 12);
  NameInput->setPositionY(3);
  NameInput->setScale(0.8f);

  auto name_input_node = NameInput->getInputNode();
  int root_touch_prio = this->getTouchPriority();

  name_input_node->setTouchPriority(root_touch_prio + 1);

  m_buttonMenu->addChild(NameInput);

  auto DoneS = CCScale9Sprite::create("GJ_button_01.png", {0, 0, 40, 40});
  DoneS->setContentSize({135, 25});
  DoneS->setScale(1);

  CCLabelBMFont* DoneSLabel = CCLabelBMFont::create("Save Backup", "bigFont.fnt");
  DoneSLabel->setScale(0.575f);
  DoneSLabel->setPosition({68, 13.5f});
  DoneS->addChild(DoneSLabel);

  auto DoneButton = CCMenuItemSpriteExtra::create(DoneS, nullptr, this, menu_selector(CreateBackupLayer::DoneAndSave));

  DoneButton->setPosition({0, -29});

  m_buttonMenu->addChild(DoneButton);

  this->setKeypadEnabled(true);
  this->setTouchEnabled(true);

  CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(parentLayer->m_buttonMenu);
  CCObject* obj;
  CCARRAY_FOREACH(parentLayer->list->m_listView->m_entries, obj) {
    auto cell = static_cast<BackupCell*>(obj);
    if (cell) {
      CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(cell->buttonsM);
    }
  }
  return true;
}

void CreateBackupLayer::show(CCNode* parent) {
  this->setZOrder(100);

  this->runAction(CCEaseElasticOut::create(CCScaleTo::create(0.6f, 1.1f), 0.5f));
  this->runAction(CCFadeTo::create(0.14f, 100));

  auto readBackups = file::readDirectory(Mod::get()->getSaveDir() / "Backups/Auto-Backups");

  parent->addChild(this);
}

void CreateBackupLayer::keyBackClicked() {
  parentLayer->RefreshBackupsList();
  this->setKeyboardEnabled(false);
  this->removeFromParentAndCleanup(true);
}

void CreateBackupLayer::onClose(CCObject* object) { keyBackClicked(); }

void CreateBackupLayer::DoneAndSave(CCObject* object) {
  std::string custom_backup_path = Mod::get()->getSettingValue<std::string>("Backup_Save_Path");
  // std::string custom_backup_path = "";

  if (NameInput->getString() != "") {
    auto creationTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const char* Time = std::ctime(&creationTime);
    std::string t = Time;
    std::string fileName = t;
    for (size_t i = 0; i < fileName.length(); i++) {
      if (fileName[i] == ':') {
        fileName[i] = '-';
      }
      if (fileName[i] == ' ') {
        fileName[i] = '_';
      }
    }

    fileName.erase(std::remove(fileName.begin(), fileName.end(), '\n'), fileName.cend());
    ghc::filesystem::path fullpath = !custom_backup_path.empty() ? ghc::filesystem::path(custom_backup_path) / fileName : this->parentLayer->BackupsDir / fileName;
    Result<> res = file::createDirectory(fullpath);

    std::string tempWPath = CCFileUtils::get()->getWritablePath();

    ghc::filesystem::path GDAPPDATAPATH(tempWPath);
    ghc::filesystem::copy(GDAPPDATAPATH / "CCGameManager.dat", fullpath);
    ghc::filesystem::copy(GDAPPDATAPATH / "CCGameManager2.dat", fullpath);
    ghc::filesystem::copy(GDAPPDATAPATH / "CCLocalLevels.dat", fullpath);
    ghc::filesystem::copy(GDAPPDATAPATH / "CCLocalLevels2.dat", fullpath);
    ghc::filesystem::path dataPath = fullpath / "Backup.dat";

    std::ofstream bkData(dataPath);

    bkData << NameInput->getString().c_str() << std::endl;
    fileName[0] = '-';
    bkData << fileName << std::endl;

    bkData.close();

    std::string succMessage = "The backup\n";
    succMessage += NameInput->getString();
    succMessage += "\nwas created successfully!";

    keyBackClicked();
    FLAlertLayer::create("Success!", succMessage, "OK")->show();
  } else {
    FLAlertLayer::create("No Name For Backup", "Please add a name to your backup.", "Back")->show();
  }
}
