#include <QApplication>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

// #include "pbform/SqliteStorage.h"
#include "recipe.pb.h"
#include "recipe.qtform.h" // generiert von protoc-gen-qtforms

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    // SqliteStorage storage(QStringLiteral("recipes.db"), demo::Recipe::descriptor());

    QWidget win;
    win.setWindowTitle(QStringLiteral("pbform Demo - Recipe"));

    auto *list = new QListWidget;
    list->setMaximumWidth(160);
    auto *newBtn = new QPushButton(QStringLiteral("Neu"));
    auto *left = new QVBoxLayout;
    left->addWidget(list);
    left->addWidget(newBtn);

    auto *form = new demo::RecipeForm;
    auto *saveBtn = new QPushButton(QStringLiteral("Speichern"));
    auto *delBtn = new QPushButton(QStringLiteral("Löschen"));
    auto *btns = new QHBoxLayout;
    btns->addWidget(saveBtn);
    btns->addWidget(delBtn);
    auto *right = new QVBoxLayout;
    right->addWidget(form);
    right->addStretch();
    right->addLayout(btns);

    auto *root = new QHBoxLayout(&win);
    root->addLayout(left);
    root->addLayout(right, 1);

    qint64 currentId = 0;

    auto refresh = [&] {
        // list->clear();
        // for (qint64 id : storage.ids()) {
        //     auto *item = new QListWidgetItem(QStringLiteral("#%1").arg(id));
        //     item->setData(Qt::UserRole, id);
        //     list->addItem(item);
        //     if (id == currentId)
        //         list->setCurrentItem(item);
        // }
    };

    QObject::connect(list, &QListWidget::currentItemChanged, [&](QListWidgetItem *it, QListWidgetItem *) {
        if (!it)
            return;
        currentId = it->data(Qt::UserRole).toLongLong();
        demo::Recipe r;
        // if (storage.load(currentId, r))
        //     form->setMessage(r);
    });

    QObject::connect(newBtn, &QPushButton::clicked, [&] {
        currentId = 0;
        list->setCurrentRow(-1);
        form->setMessage(demo::Recipe{});
    });

    // QObject::connect(saveBtn, &QPushButton::clicked, [&] {
    //     demo::Recipe r;
    //     form->getMessage(r);
    //     currentId = storage.save(r, currentId);
    //     refresh();
    // });

    // QObject::connect(delBtn, &QPushButton::clicked, [&] {
    //     if (currentId == 0)
    //         return;
    //     storage.remove(currentId);
    //     currentId = 0;
    //     refresh();
    //     form->setMessage(demo::Recipe{});
    // });

    form->setMessage(demo::Recipe{});
    refresh();
    win.show();
    return app.exec();
}
