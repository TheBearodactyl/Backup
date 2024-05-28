#include "BackupCell.h"
#include <RenameBackupLayer.h>
#include <ghc/filesystem.hpp>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>

BackupCell* BackupCell::create(ghc::filesystem::path folderPath, BackupsLayer* mainLayer, bool autos, CCSize cellSize) {
  BackupCell* ret = new BackupCell();
  if (ret && ret->init(folderPath, mainLayer, autos, cellSize)) {
    ret->autorelease();
  } else {
    delete ret;
    ret = nullptr;
  }
  return ret;
}

bool BackupCell::init(ghc::filesystem::path folderPath, BackupsLayer* _mainLayer, bool _autos, CCSize cellSize) {
  if (_autos) {
    CCSprite* icon = CCSprite::createWithSpriteFrameName("diffIcon_auto_btn_001.png");
    icon->setPosition({23, 20});
    this->addChild(icon);
  } else {
    CCSprite* icon = CCSprite::createWithSpriteFrameName("folderIcon_001.png");
    icon->setPosition({23, 20});
    this->addChild(icon);
  }

  _folderPath = folderPath;
  mainLayer = _mainLayer;
  autos = _autos;

  std::ifstream dataFile;
  ghc::filesystem::path dataPath = folderPath / "Backup.dat";
  dataFile.open(dataPath);

  if (dataFile) {
    std::string dataFileData;
    std::string line;
    int step = 0;

    while (std::getline(dataFile, line)) {
      if (step == 0) {
        Name = line;
      } else if (step == 1) {
        DateOfCreation = line;
        break;
      }
      step++;
    }

    // Format DateOfCreation string
    for (char& c : DateOfCreation) {
      if (c == '_') {
        c = ' ';
      } else if (c == '-') {
        c = ':';
      }
    }

    size_t backup_size = calc_dir_size(folderPath);
    std::string backup_size_str = format_size(backup_size);
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    buttonsM = CCMenu::create();
    buttonsM->setPositionX(buttonsM->getPositionX() * (569 / winSize.width));

    CCLabelBMFont* NameLabel = CCLabelBMFont::create(Name.c_str(), "bigFont.fnt");
    CCLabelBMFont* DateOfCreationLabel = CCLabelBMFont::create(DateOfCreation.c_str(), "bigFont.fnt");
    CCLabelBMFont* size_label = CCLabelBMFont::create(backup_size_str.c_str(), "bigFont.fnt");
    CCSprite* deleteBSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    CCMenuItemSpriteExtra* deleteButton = CCMenuItemSpriteExtra::create(deleteBSprite, nullptr, this, menu_selector(BackupCell::DeleteMe));

    NameLabel->setAnchorPoint(ccp(0.0f, 0.5f));
    NameLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    NameLabel->setPosition({46, 30});
    NameLabel->setScale(0.45f);
    DateOfCreationLabel->setAnchorPoint(ccp(0.0f, 0.5f));
    DateOfCreationLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    DateOfCreationLabel->setOpacity(135);
    DateOfCreationLabel->setPosition({47, 16});
    DateOfCreationLabel->setScale(0.325f);
    size_label->setAnchorPoint(ccp(0.0f, 0.5f));
    size_label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    size_label->setOpacity(135);
    size_label->setPosition({NameLabel->getPositionX() + 90, 30});
    size_label->setScale(0.325f);
    deleteBSprite->setScale(0.65f);
    deleteButton->setPosition({-20, -140});

    this->addChild(NameLabel);
    this->addChild(size_label);
    this->addChild(DateOfCreationLabel);
    this->addChild(buttonsM);

    // delete backup button
    buttonsM->addChild(deleteButton);

    // load backup button
    auto loadeBSprite = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
    loadeBSprite->setScale(0.70f);
    auto loadButton = CCMenuItemSpriteExtra::create(loadeBSprite, nullptr, this, menu_selector(BackupCell::LoadBackup));
    loadButton->setPosition({-49, -140});
    buttonsM->addChild(loadButton);

    // edit backup button
    auto editBSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn02_001.png");
    editBSprite->setScale(0.675f);
    auto editButton = CCMenuItemSpriteExtra::create(editBSprite, nullptr, this, menu_selector(BackupCell::EditBackup));
    editButton->setPosition({-78, -140});
    buttonsM->addChild(editButton);

    auto toggleOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    auto toggleOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");

    auto toggleSelect = CCMenuItemToggler::create(toggleOff, toggleOn, this, menu_selector(BackupCell::OnSelectedStateChanged));
    toggleSelect->setPosition({7, -140});
    toggleSelect->setScale(0.6f);
    buttonsM->addChild(toggleSelect);
  }

  return true;
}

size_t BackupCell::calc_dir_size(const ghc::filesystem::path& dir_path) {
  size_t size = 0;

  for (const auto& entry : ghc::filesystem::recursive_directory_iterator(dir_path)) {
    if (ghc::filesystem::is_regular_file(entry.status())) {
      size += ghc::filesystem::file_size(entry.path());
    }
  }

  return size;
}

std::string BackupCell::format_size(size_t size) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(1) << "(" << (size >= (1 << 30) ? (size / static_cast<double>(1 << 30)) : size >= (1 << 20) ? (size / static_cast<double>(1 << 20)) : size >= (1 << 10) ? (size / static_cast<double>(1 << 10)) : size) << (size >= (1 << 30) ? " GB" : size >= (1 << 20) ? " MB" : size >= (1 << 10) ? " KB" : " B") << ")";
  return ss.str();
}

void BackupCell::DeleteMe(CCObject* object) {
  deleteWarning = FLAlertLayer::create(this, "Warning!", "Are you sure you want\nto delete \"" + Name + "\"?", "No", "Yes");
  deleteWarning->show();
}

void BackupCell::LoadBackup(CCObject* object) {
  std::string message = "";
#ifdef GEODE_IS_ANDROID
  message = "Are you sure you want\nto apply \"" + Name +
            "\"? \nthis backup will replace your\n current save.\nthis will "
            "also close down your game.";
#else
  message = "Are you sure you want\nto apply \"" + Name +
            "\"? \nthis backup will replace your\n current save.\nthis will "
            "also restart your game.";
#endif
  loadWarning = FLAlertLayer::create(this, "Warning!", message.c_str(), "No", "Yes");
  loadWarning->show();
}

void BackupCell::EditBackup(CCObject* object) { RenameBackupLayer::create(mainLayer, this)->show(mainLayer); }

void BackupCell::OnSelectedStateChanged(CCObject* object) { selected = !selected; }

void BackupCell::FLAlert_Clicked(FLAlertLayer* p0, bool p1) {
  if (p0 == deleteWarning) {
    if (p1) {
      ghc::filesystem::remove_all(_folderPath);
      mainLayer->RefreshBackupsList();
    }
  }
  if (p0 == loadWarning) {
    if (p1) {
      std::string tempWPath = CCFileUtils::get()->getWritablePath();
      ghc::filesystem::path GDAPPDATAPATH(tempWPath);
      geode::Result<> res;
      res = geode::utils::file::writeString(GDAPPDATAPATH / "CCGameManager.dat", geode::utils::file::readString(_folderPath / "CCGameManager.dat").value());
      res = geode::utils::file::writeString(GDAPPDATAPATH / "CCGameManager2.dat", geode::utils::file::readString(_folderPath / "CCGameManager2.dat").value());

      res = geode::utils::file::writeString(GDAPPDATAPATH / "CCLocalLevels.dat", geode::utils::file::readString(_folderPath / "CCLocalLevels.dat").value());
      res = geode::utils::file::writeString(GDAPPDATAPATH / "CCLocalLevels2.dat", geode::utils::file::readString(_folderPath / "CCLocalLevels2.dat").value());

#ifdef GEODE_IS_ANDROID
#else
      game::restart();
#endif

      exit(0);
    }
  }
}
