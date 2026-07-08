#include <gtest/gtest.h>

#include "substitutionstable.h"
#include <QStringListModel>
#include <QComboBox>
#include <QTableWidgetItem>
#include <iostream>

TEST(SubstitutionsTableTest, EmptyTableReturnsEmptySubstitutions) {
  SubstitutionsTable table;
  QStringListModel model;
  table.setSourceLanguageModel(&model);

  auto subs = table.substitutions();
  EXPECT_TRUE(subs.empty());
}

TEST(SubstitutionsTableTest, ValidRowIsReturned) {
  SubstitutionsTable table;
  QStringListModel model;
  table.setSourceLanguageModel(&model);

  Substitutions initial;
  initial.emplace("eng", Substitution{"source_text", "target_text"});
  table.setSubstitutions(initial);

  auto subs = table.substitutions();
  EXPECT_EQ(subs.size(), 1u);

  auto it = subs.find("eng");
  ASSERT_NE(it, subs.end());
  EXPECT_EQ(it->second.source, "source_text");
  EXPECT_EQ(it->second.target, "target_text");
}

TEST(SubstitutionsTableTest, EmptyLanguageIsIgnored) {
  SubstitutionsTable table;
  QStringListModel model;
  table.setSourceLanguageModel(&model);

  Substitutions initial;
  initial.emplace("eng", Substitution{"source_text", "target_text"});
  table.setSubstitutions(initial);

  auto combo = static_cast<QComboBox*>(table.cellWidget(0, static_cast<int>(SubstitutionsTable::Column::Language)));
  ASSERT_TRUE(combo != nullptr);

  // Set the current text explicitly to empty string so combo->currentText() returns empty
  combo->setEditable(true);
  combo->setEditText("");

  auto subs = table.substitutions();
  EXPECT_TRUE(subs.empty());
}

TEST(SubstitutionsTableTest, EmptySourceIsIgnored) {
  SubstitutionsTable table;
  QStringListModel model;
  table.setSourceLanguageModel(&model);

  Substitutions initial;
  initial.emplace("eng", Substitution{"", "target_text"});
  table.setSubstitutions(initial);

  auto subs = table.substitutions();
  EXPECT_TRUE(subs.empty());
}

TEST(SubstitutionsTableTest, MultipleRows) {
  SubstitutionsTable table;
  QStringListModel model;
  table.setSourceLanguageModel(&model);

  Substitutions initial;
  initial.emplace("eng", Substitution{"source1", "target1"});
  initial.emplace("rus", Substitution{"source2", "target2"});
  initial.emplace("fra", Substitution{"", "target3"});     // Empty source
  initial.emplace("deu", Substitution{"source4", "target4"});
  table.setSubstitutions(initial);

  // Clear language for the "rus" row (index 1).
  // Wait, initial is a multimap, it iterates in sorted order of LanguageId.
  // The keys inserted are: "deu", "eng", "fra", "rus".
  // So the row indices are:
  // 0: deu
  // 1: eng
  // 2: fra
  // 3: rus
  // Let's clear row 3 which is "rus".

  auto combo = static_cast<QComboBox*>(table.cellWidget(3, static_cast<int>(SubstitutionsTable::Column::Language)));
  ASSERT_TRUE(combo != nullptr);
  combo->setEditable(true);
  combo->setEditText("");

  auto subs = table.substitutions();
  EXPECT_EQ(subs.size(), 2u);

  auto it1 = subs.find("eng");
  ASSERT_NE(it1, subs.end());
  if (it1 != subs.end()) {
    EXPECT_EQ(it1->second.source, "source1");
    EXPECT_EQ(it1->second.target, "target1");
  }

  auto it2 = subs.find("deu");
  ASSERT_NE(it2, subs.end());
  if (it2 != subs.end()) {
    EXPECT_EQ(it2->second.source, "source4");
    EXPECT_EQ(it2->second.target, "target4");
  }
}
