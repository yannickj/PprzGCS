#include "settings_viewer.h"
#include "dispatcher_ui.h"
#include "pprz_dispatcher.h"
#include "AircraftManager.h"
#include "double_slider.h"
#include "switch.h"
#include <QDebug>

SettingsViewer::SettingsViewer(QString ac_id, QWidget *parent) : QWidget(parent), ac_id(ac_id)
{
    main_layout = new QVBoxLayout(this);
    search_layout = new QHBoxLayout();
    path_layout = new QHBoxLayout();
    main_layout->addItem(search_layout);


    button_home = new QToolButton(this);
    button_home->setText(QString::fromUtf8("\xE2\x8C\x82"));

    path_layout->addWidget(button_home);
    path = new QStackedWidget(this);
    path_layout->addWidget(path);

    main_layout->addItem(path_layout);

    scroll = new QScrollArea();
    scroll_content = new QStackedWidget();

    main_layout->addWidget(scroll);
    scroll->setWidget(scroll_content);
    scroll->setWidgetResizable(true);

    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    search_bar = new QLineEdit(this);
    search_bar->setPlaceholderText("search");
    search_layout->addWidget(search_bar);

    auto search_callback = [=](const QString str) {
        if(str == "") {
            restore_searched_items();
            scroll_content->setCurrentIndex(last_widget_index);
            path->setCurrentIndex(last_path_index);
        } else {
            populate_search_results(str);
            scroll_content->setCurrentIndex(search_result_index);
            path->setCurrentIndex(search_path_index);
        }
    };

    connect(
        search_bar, &QLineEdit::textChanged,
        search_callback
    );

    connect(
        search_bar, &QLineEdit::returnPressed,
        [=]() {
            auto str = search_bar->text();
            search_callback(str);
    }
    );

    //setStyleSheet("QWidget{background-color: #31363b;} QLabel{color:white;} QAbstractButton{color:white;} QLineEdit{color:white;}");
    //setAutoFillBackground(true);

    connect(DispatcherUi::get(), &DispatcherUi::settingUpdated, this, &SettingsViewer::updateSettings);

    init(ac_id);
}

bool SettingsViewer::eventFilter(QObject *object, QEvent *event)
{
    (void)object;
    (void)event;
    if(search_bar->hasFocus() && event->type() == QEvent::KeyPress) {
        return true;    //interrupt event
    }
    return false;
}

void SettingsViewer::init(QString ac_id) {
    this->ac_id = ac_id;
    auto settings = AircraftManager::get()->getAircraft(ac_id).getSettingMenu();
    create_widgets(settings, QList<shared_ptr<SettingMenu>>());
    last_widget_index = widgets_indexes[settings];
    last_path_index = path_indexes[settings];

    auto result_widget = new QWidget(scroll_content);
    new QVBoxLayout(result_widget);
    result_widget->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    search_result_index = scroll_content->addWidget(result_widget);
    auto search_label = new QLabel("  searching...", path);
    search_path_index = path->addWidget(search_label);


    scroll_content->setCurrentIndex(widgets_indexes[settings]);

    connect(
        button_home, &QToolButton::clicked,
        [=]() {
            scroll_content->setCurrentIndex(widgets_indexes[settings]);
            path->setCurrentIndex(path_indexes[settings]);
            scroll->verticalScrollBar()->setValue(0);
        }
    );



}

void SettingsViewer::create_widgets(shared_ptr<SettingMenu> setting_menu, QList<shared_ptr<SettingMenu>> stack) {
    auto widget = new QWidget(scroll_content);
    auto path_widget = new QWidget(this);
    auto menu_layout = new QVBoxLayout(widget);
    auto current_path_layout = new QHBoxLayout(path_widget);

    auto new_stack = QList<shared_ptr<SettingMenu>>(stack);
    new_stack.append(setting_menu);

    if(new_stack.size()>0) {
        for(auto setmm = std::next(new_stack.begin()); setmm != new_stack.end(); ++setmm) {
            auto setm = (*setmm);
            auto sep = new QLabel(">", path_widget);
            auto button = new QPushButton(setm->getName().c_str(), path_widget);
            sep->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
            button->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
            current_path_layout->addWidget(sep);
            current_path_layout->addWidget(button);

            connect(
                button, &QPushButton::clicked,
                [=]() {

                    scroll_content->setCurrentIndex(widgets_indexes[setm]);
                    path->setCurrentIndex(path_indexes[setm]);
                    restore_searched_items();
                    last_widget_index = widgets_indexes[setm];
                    last_path_index = path_indexes[setm];
                    scroll->verticalScrollBar()->setValue(0);
                }
            );
        }

        current_path_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    }

    int index_path = path->addWidget(path_widget);
    path_indexes[setting_menu] = index_path;

    for(auto sets: setting_menu->getSettingMenus()) {
        auto button = new QPushButton(sets->getName().c_str());
        menu_layout->addWidget(button);

        setting_menu_widgets[sets] = button;

        create_widgets(sets, new_stack);
        connect(
            button, &QPushButton::clicked,
            [=]() {
                scroll_content->setCurrentIndex(widgets_indexes[sets]);
                path->setCurrentIndex(path_indexes[sets]);
                restore_searched_items();
                last_widget_index = widgets_indexes[sets];
                last_path_index = path_indexes[sets];
                scroll->verticalScrollBar()->setValue(0);
            }
        );
    }
    for(auto set: setting_menu->getSettings()) {
        auto setting_widget = makeSettingWidget(set, widget);
        menu_layout->addWidget(setting_widget);
        setting_widgets[set] = setting_widget;
    }
    menu_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    int index_widget = scroll_content->addWidget(widget);
    widgets_indexes[setting_menu] = index_widget;
}

void SettingsViewer::restore_searched_items() {
    while (pos_hist.size() > 0) {
        auto pos = pos_hist.takeLast();
        pos.layout->insertWidget(pos.index, pos.widget);
    }
}

void SettingsViewer::populate_search_results(QString searched) {
    restore_searched_items();

    QBoxLayout* l = dynamic_cast<QBoxLayout*>(scroll_content->widget(search_result_index)->layout());

    // search in Menus
    for(auto sets: setting_menu_widgets.keys()) {
        QString name = sets->getName().c_str();
        if(name.toLower().contains(searched.toLower())) {
            auto w = setting_menu_widgets[sets];
            int index = w->parentWidget()->layout()->indexOf(w);
            pos_hist.push_back(PositionHistory{w, dynamic_cast<QBoxLayout*>(w->parentWidget()->layout()), index});
            l->insertWidget(l->count()-1, w);
        }
    }

    //search in settings
    for(auto set: setting_widgets.keys()) {
        QString name = set->getName().c_str();
        if(name.toLower().contains(searched.toLower())) {
            auto w = setting_widgets[set];
            int index = w->parentWidget()->layout()->indexOf(w);
            pos_hist.push_back(PositionHistory{w, dynamic_cast<QBoxLayout*>(w->parentWidget()->layout()), index});
            l->insertWidget(l->count()-1, w);
        }
    }


}



QWidget* SettingsViewer::makeSettingWidget(shared_ptr<Setting> setting, QWidget* parent) {
    QWidget* widget = new QWidget(parent);
    QVBoxLayout* vLay = new QVBoxLayout(widget);
    QHBoxLayout* hlay = new QHBoxLayout();
    vLay->addItem(hlay);

    QWidget* label = new QLabel(setting->getName().c_str(), widget);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hlay->addWidget(label);

    QPushButton* value_btn = new QPushButton("?", widget);
    value_btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hlay->addWidget(value_btn);
    auto ok_btn = new QToolButton(widget);
    ok_btn->setText(QString::fromUtf8("\xE2\x9C\x93"));
    ok_btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ok_btn->setToolTip("commit");
    hlay->addWidget(ok_btn);

    auto undo_btn = new QToolButton(widget);
    undo_btn->setText(QString::fromUtf8("\xE2\x86\xA9"));
    undo_btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    undo_btn->setToolTip("undo");
    hlay->addWidget(undo_btn);

    auto reset_btn = new QToolButton(widget);
    reset_btn->setText(QString::fromUtf8("\xE2\x86\xBA"));
    reset_btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    reset_btn->setToolTip("reset to initial");
    hlay->addWidget(reset_btn);

    connect(value_btn, &QPushButton::clicked, [=]() {
        qDebug() << "setting " << setting->getNo() << " of AC " << ac_id << " clicked !";
        value_btn->setText("?");
        pprzlink::Message getSetting(PprzDispatcher::get()->getDict()->getDefinition("GET_DL_SETTING"));
        getSetting.addField("ac_id", ac_id.toStdString());
        getSetting.addField("index", setting->getNo());
        PprzDispatcher::get()->sendMessage(getSetting);
    });

    connect(undo_btn, &QToolButton::clicked, [=]() {
        auto prev = setting->getPreviousValue();
        AircraftManager::get()->getAircraft(ac_id).setSetting(setting, prev);
        setting->setValue(prev);
    });

    connect(reset_btn, &QToolButton::clicked, [=]() {
        auto initial_value = setting->getInitialValue();
        if(initial_value.has_value()) {
            AircraftManager::get()->getAircraft(ac_id).setSetting(setting, initial_value.value());
            setting->setValue(initial_value.value());
        }

    });

    initialized[setting] = false;

    auto [min, max, step] = setting->getBounds();

    if(setting->getValues().size() > 2) {
        // named values -> combo box
        QComboBox* combo = new QComboBox(widget);
        for(string s:setting->getValues()) {
            combo->addItem(s.c_str());
        }
        vLay->addWidget(combo);

        setters[setting] = [combo](double value) {combo->setCurrentIndex(static_cast<int>(value));};
        label_setters[setting] = [combo, value_btn](double value) {
            value_btn->setText(combo->itemText(static_cast<int>(value)));
        };

        connect(ok_btn, &QToolButton::clicked, [=]() {
            float value = static_cast<float>(combo->currentIndex());
            AircraftManager::get()->getAircraft(ac_id).setSetting(setting, value);
            setting->setValue(value);
        });

    } else if(setting->getValues().size() == 2 || abs(min+step-max) < 0.0001) {
        QString s1 = setting->getValues().size() == 2 ? setting->getValues()[0].c_str() : QString::number(min);
        QString s2 = setting->getValues().size() == 2 ? setting->getValues()[1].c_str() : QString::number(max);
        QHBoxLayout *hbox = new QHBoxLayout;
        Switch* sw = new Switch();
        QLabel* l1 = new QLabel(s1);
        l1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QLabel* l2 = new QLabel(s2);
        l2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        hbox->addWidget(l1);
        hbox->addWidget(sw);
        hbox->addWidget(l2);

        vLay->addLayout(hbox);


        setters[setting] = [sw](double value) {sw->setChecked(value > 0.5);};
        label_setters[setting] = [setting, value_btn, s1, s2](double value) {
            QString txt;
            if(setting->getValues().size() == 2) {
                txt = value < 0.5 ? s1: s2;
            } else {
                txt = QString::number(value);
            }

            value_btn->setText(txt);
        };

        connect(ok_btn, &QToolButton::clicked, [=]() {
            float value = static_cast<float>(sw->isChecked());
            AircraftManager::get()->getAircraft(ac_id).setSetting(setting, value);
            setting->setValue(value);
        });

    } else if(abs(max-min) < 0.00001) {
        setters[setting] = [](double value) {(void)value;};
        label_setters[setting] = [value_btn](double value) {value_btn->setText(QString::number(value));};
        float value = min;
        QLabel* uniq_val = new QLabel(QString::number(value));
        uniq_val->setAlignment(Qt::AlignCenter);
        vLay->addWidget(uniq_val);

        connect(ok_btn, &QToolButton::clicked, [=]() {
            AircraftManager::get()->getAircraft(ac_id).setSetting(setting, value);
            setting->setValue(value);
        });

    } else {
        DoubleSlider* slider = new DoubleSlider(Qt::Horizontal, widget);
        QLineEdit* raw_edit = new QLineEdit(widget);
        raw_edit->hide();
        QToolButton* expert_button = new QToolButton(widget);
        expert_button->setIcon(QIcon(":/pictures/lock_dark.svg"));
        expert_button->setToolTip("Expert mode");

        setters[setting] = [slider, raw_edit](double value) {slider->setDoubleValue(value); raw_edit->setText(QString::number(value));};
        label_setters[setting] = [value_btn](double value) {value_btn->setText(QString::number(value));};

        slider->setDoubleRange(min, max, step);
        QHBoxLayout* vbox = new QHBoxLayout();
        QLabel* la = new QLabel(QString::number(min, 'f', 2));
        int precision = 0;
        if(step < 1) {
            precision = static_cast<int>(ceil(abs(log10(step))));
        }
        connect(slider, &DoubleSlider::doubleValueChanged,
            [=](double value) {
                la->setText(QString::number(value, 'f', precision));
            });

        connect(expert_button, &QToolButton::clicked,
            [=]() {
                raw_edit->setVisible(!raw_edit->isVisible());
                if(raw_edit->isVisible()) {
                    expert_button->setIcon(QIcon(":/pictures/lock_warning.svg"));
                } else {
                    expert_button->setIcon(QIcon(":/pictures/lock_dark.svg"));
                }
            });

        connect(raw_edit, &QLineEdit::returnPressed,
            [=, min=min, max=max]() {
                auto txt = raw_edit->text();
                bool ok;
                auto value = static_cast<float>(txt.toDouble(&ok));
                if(ok) {
                    if(value > min && value < max) {
                        raw_edit->setStyleSheet("QLineEdit{background-color: #88ff88;}");
                    } else {
                        raw_edit->setStyleSheet("QLineEdit{background-color: #ffd088;}");
                    }
                    AircraftManager::get()->getAircraft(ac_id).setSetting(setting, value);
                } else {
                    raw_edit->setStyleSheet("QLineEdit{background-color: #ff8888;}");
                }
            });

        connect(raw_edit, &QLineEdit::textChanged,
            [=]() {
                raw_edit->setStyleSheet("QLineEdit{background-color: #ffffff;}");
            });

        vbox->addWidget(la);
        la->setAlignment(Qt::AlignCenter);
        vbox->addWidget(slider);
        vbox->addWidget(raw_edit);
        vbox->addWidget(expert_button);
        vLay->addLayout(vbox);

        connect(ok_btn, &QToolButton::clicked, [=]() {
            auto coef = setting->getAltUnitCoef();
            float value = static_cast<float>(slider->doubleValue()) / coef;
            AircraftManager::get()->getAircraft(ac_id).setSetting(setting, value);
            setting->setValue(value);
        });
    }

    QFrame *line = new QFrame(widget);
    vLay->addWidget(line);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return widget;
}

void SettingsViewer::updateSettings(QString id, shared_ptr<Setting> setting, float value) {
    if(ac_id == id) {
        auto val = setting->getAltUnitCoef() * value;
        label_setters[setting](val);
        if(!initialized[setting]) {
            setters[setting](val);
            setting->setInitialValue(value);
            initialized[setting] = true;
        }
    }
}
