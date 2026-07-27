#include "testrunner.h"
#include "controls.h"
#include "arena.h"

static mem_Arena arena;
static ui_Window win;
static ui_Wrapper *ctl;

static ui_Wrapper* create_ctrl(enum ui_ControlType type)
{
    ui_Wrapper *ctl = ui_window_new_control(&win, type);
    ui_window_append(&win, ctl);
    return ctl;
}

TEST_SETUP(ctl_suite)

TEST_SUITE_SETUP_FN(ctl_suite)
{
    mem_arena_init(&arena);
}

TEST_SUITE_TEARDOWN_FN(ctl_suite)
{
    mem_arena_free(&arena);
}

TEST_SETUP_FN(ctl_suite)
{
    ui_window_init(&win, &arena);
    ctl = NULL;
}

TEST(ctl_suite, win_init, "Should init")
{
    memset(&win, 0x5A, sizeof(ui_Window));
    ui_window_init(&win, &arena);
    expectEQ((void*)win.arena, &arena);
    expectEQ((void*)win.root, NULL);
    expectEQ((void*)win.focus_control, NULL);
    expectEQ((void*)win.first_tab_order, NULL);
}

TEST(ctl_suite, win_create_lbl, "Test create label")
{
    ctl = ui_window_new_control(&win, UI_LabelType);
    expectEQ(ctl->label->name, "Label");
    expectNE((void*)ctl, NULL);
    expectEQ((void*)ctl->window, &win);
    expectEQ((void*)ctl->parent, NULL);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl->prev_sibling, NULL);
    expectEQ((void*)ctl->first_child, NULL);
    expectEQ(ctl->type, UI_LabelType);
    expectTrue(ctl->dirty);
    expectEQ(ctl->rect.top_left.x, 0);
    expectEQ(ctl->rect.top_left.y, 0);
    expectEQ(ctl->rect.bottom_right.x, 0);
    expectEQ(ctl->rect.bottom_right.y, 0);
    expectNE(ctl->label->bg_color, 0);
    expectNE(ctl->label->fg_color, 0);
    expectEQ(ctl->label->format, 0);
    expectEQ((void*)ctl->label->wrapper, ctl);
    expectEQ(ctl->label->shown, true);
    expectEQ((void*)ctl->label->text.arena, &arena);
}


TEST(ctl_suite, win_create_btn, "Test create Button")
{
    ctl = ui_window_new_control(&win, UI_ButtonType);
    expectEQ(ctl->button->name, "Button");
    expectNE((void*)ctl, NULL);
    expectEQ((void*)ctl->window, &win);
    expectEQ((void*)ctl->parent, NULL);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl->prev_sibling, NULL);
    expectEQ((void*)ctl->first_child, NULL);
    expectEQ(ctl->type, UI_ButtonType);
    expectTrue(ctl->dirty);
    expectEQ(ctl->rect.top_left.x, 0);
    expectEQ(ctl->rect.top_left.y, 0);
    expectEQ(ctl->rect.bottom_right.x, 0);
    expectEQ(ctl->rect.bottom_right.y, 0);
    expectNE(ctl->button->bg_color, 0);
    expectNE(ctl->button->fg_color, 0);
    expectEQ(ctl->button->format, 0);
    expectEQ((void*)ctl->button->wrapper, ctl);
    expectEQ(ctl->button->shown, true);
    expectEQ((void*)ctl->button->text.arena, &arena);
}

TEST(ctl_suite, win_insert, "Test insert")
{
    ui_Wrapper *ctl1, *ctl2, *ctl3;
    ctl = ui_window_new_control(&win, UI_LabelType);
    ctl1 = ui_window_new_control(&win, UI_TextEditType);
    ctl2 = ui_window_new_control(&win, UI_ContainerType);
    ctl3 = ui_window_new_control(&win, UI_ListType);

    ui_window_insert(&win, ctl, 0);
    expectEQ((void*)win.root, ctl);
    expectEQ((void*)ctl->parent, NULL);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl->first_child, NULL);

    // 0=textedit, 1=label
    ui_window_insert(&win, ctl1, 0);
    expectEQ((void*)win.root, ctl1);
    expectEQ((void*)ctl->parent, NULL);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl1->next_sibling, ctl);
    expectEQ((void*)ctl1->parent, NULL);
    expectEQ((void*)ctl1->first_child, NULL);
    expectEQ((void*)ctl->first_child, NULL);

    // 0=textedit, 1=Container, 2=label
    ui_window_insert(&win, ctl2, 1);
    expectEQ((void*)win.root, ctl1);
    expectEQ((void*)ctl1->next_sibling, ctl2);
    expectEQ((void*)ctl2->next_sibling, ctl);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl2->first_child, NULL);
    expectEQ((void*)ctl1->first_child, NULL);
    expectEQ((void*)ctl->first_child, NULL);

    // 0=textedit, 1=Container, 2=label, 3=List
    ui_window_insert(&win, ctl3, -1);
    expectEQ((void*)win.root, ctl1);
    expectEQ((void*)ctl->next_sibling, ctl3);
    expectEQ((void*)ctl3->next_sibling, NULL);
    expectEQ((void*)ctl3->first_child, NULL);
    expectEQ((void*)ctl2->first_child, NULL);
    expectEQ((void*)ctl1->first_child, NULL);
    expectEQ((void*)ctl->first_child, NULL);
}

TEST(ctl_suite, win_append, "Should append")
{
    ui_Wrapper *ctl1, *ctl2, *ctl3;
    ctl = ui_window_new_control(&win, UI_LabelType);
    ctl1 = ui_window_new_control(&win, UI_TextEditType);
    ctl2 = ui_window_new_control(&win, UI_ContainerType);
    ctl3 = ui_window_new_control(&win, UI_ListType);
    ui_window_append(&win, ctl);
    ui_window_append(&win, ctl1);
    ui_window_append(&win, ctl2);
    ui_window_append(&win, ctl3);
    expectEQ((void*)win.root, ctl);
    expectEQ((void*)ctl->next_sibling, ctl1);
    expectEQ((void*)ctl->prev_sibling, NULL);
    expectEQ((void*)ctl1->next_sibling, ctl2);
    expectEQ((void*)ctl1->prev_sibling, ctl);
    expectEQ((void*)ctl2->next_sibling, ctl3);
    expectEQ((void*)ctl2->prev_sibling, ctl1);
    expectEQ((void*)ctl3->next_sibling, NULL);
    expectEQ((void*)ctl3->prev_sibling, ctl2);
}

TEST(ctl_suite, win_remove_ctl, "Test remove")
{
    ui_Wrapper *ctl1, *ctl2, *ctl3;
    ctl = ui_window_new_control(&win, UI_LabelType);
    ctl1 = ui_window_new_control(&win, UI_TextEditType);
    ctl2 = ui_window_new_control(&win, UI_ContainerType);
    ctl3 = ui_window_new_control(&win, UI_ListType);
    ui_window_append(&win, ctl);
    ui_window_append(&win, ctl1);
    ui_window_append(&win, ctl2);
    ui_window_append(&win, ctl3);

    ui_window_remove(&win, ctl3);
    expectEQ((void*)ctl2->next_sibling, NULL);
    expectEQ((void*)ctl2->prev_sibling, ctl1);
    expectEQ((void*)win.root, ctl);

    ui_window_remove(&win, ctl);
    expectEQ((void*)win.root, ctl1);
    expectEQ((void*)ctl1->prev_sibling, NULL);

    ui_window_remove(&win, ctl2);
    expectEQ((void*)ctl1->next_sibling, NULL);
    expectEQ((void*)ctl1->prev_sibling, NULL);
    expectEQ((void*)win.root, ctl1);

    ui_window_remove(&win, ctl1);
    expectEQ((void*)win.root, NULL);

}

TEST(ctl_suite, cont_insert, "Test control insert")
{
    ui_Wrapper *cont, *ctl1, *ctl2, *ctl3;
    cont = ui_window_new_control(&win, UI_ContainerType);
    ctl = ui_window_new_control(&win, UI_LabelType);
    ctl1 = ui_window_new_control(&win, UI_TextEditType);
    ctl2 = ui_window_new_control(&win, UI_ContainerType);
    ctl3 = ui_window_new_control(&win, UI_ListType);

    ui_control_insert(cont, ctl, 0);
    expectEQ((void*)cont->first_child, ctl);
    expectEQ((void*)ctl->parent, cont);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl->prev_sibling, NULL);
    expectEQ((void*)ctl->first_child, NULL);

    // 0=textedit, 1=label
    ui_control_insert(cont, ctl1, 0);
    expectEQ((void*)cont->first_child, ctl1);
    expectEQ((void*)ctl->parent, cont);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl->prev_sibling, ctl1);
    expectEQ((void*)ctl1->next_sibling, ctl);
    expectEQ((void*)ctl1->prev_sibling, NULL);
    expectEQ((void*)ctl1->parent, cont);
    expectEQ((void*)ctl1->first_child, NULL);
    expectEQ((void*)ctl->first_child, NULL);

    // 0=textedit, 1=Container, 2=label
    ui_control_insert(cont, ctl2, 1);
    expectEQ((void*)cont->first_child, ctl1);
    expectEQ((void*)ctl1->next_sibling, ctl2);
    expectEQ((void*)ctl1->prev_sibling, NULL);
    expectEQ((void*)ctl2->next_sibling, ctl);
    expectEQ((void*)ctl2->prev_sibling, ctl1);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl->prev_sibling, ctl2);
    expectEQ((void*)ctl2->first_child, NULL);
    expectEQ((void*)ctl1->first_child, NULL);
    expectEQ((void*)ctl->first_child, NULL);

    // 0=textedit, 1=Container, 2=label, 3=List
    ui_control_insert(cont, ctl3, -1);
    expectEQ((void*)cont->first_child, ctl1);
    expectEQ((void*)ctl->next_sibling, ctl3);
    expectEQ((void*)ctl3->next_sibling, NULL);
    expectEQ((void*)ctl3->prev_sibling, ctl);
    expectEQ((void*)ctl3->first_child, NULL);
    expectEQ((void*)ctl2->first_child, NULL);
    expectEQ((void*)ctl1->first_child, NULL);
    expectEQ((void*)ctl->first_child, NULL);
}

TEST(ctl_suite, cont_append, "Should control append")
{
    ui_Wrapper *cont, *ctl1, *ctl2, *ctl3;
    cont = ui_window_new_control(&win, UI_ContainerType);
    ctl = ui_window_new_control(&win, UI_LabelType);
    ctl1 = ui_window_new_control(&win, UI_TextEditType);
    ctl2 = ui_window_new_control(&win, UI_ContainerType);
    ctl3 = ui_window_new_control(&win, UI_ListType);
    ui_control_append(cont, ctl);
    ui_control_append(cont, ctl1);
    ui_control_append(cont, ctl2);
    ui_control_append(cont, ctl3);
    expectEQ((void*)cont->first_child, ctl);
    expectEQ((void*)ctl->next_sibling, ctl1);
    expectEQ((void*)ctl1->next_sibling, ctl2);
    expectEQ((void*)ctl2->next_sibling, ctl3);
    expectEQ((void*)ctl3->next_sibling, NULL);
}

TEST(ctl_suite, control_remove_ctl, "Test control remove")
{
    ui_Wrapper *cont, *ctl1, *ctl2, *ctl3;
    cont = ui_window_new_control(&win, UI_ContainerType);
    ctl = ui_window_new_control(&win, UI_LabelType);
    ctl1 = ui_window_new_control(&win, UI_TextEditType);
    ctl2 = ui_window_new_control(&win, UI_ContainerType);
    ctl3 = ui_window_new_control(&win, UI_ListType);
    ui_control_append(cont, ctl);
    ui_control_append(cont, ctl1);
    ui_control_append(cont, ctl2);
    ui_control_append(cont, ctl3);

    ui_control_remove(cont, ctl3);
    expectEQ((void*)ctl2->next_sibling, NULL);
    expectEQ((void*)cont->first_child, ctl);

    ui_control_remove(cont, ctl);
    expectEQ((void*)cont->first_child, ctl1);

    ui_control_remove(cont, ctl2);
    expectEQ((void*)ctl1->next_sibling, NULL);
    expectEQ((void*)cont->first_child, ctl1);

    ui_control_remove(cont, ctl1);
    expectEQ((void*)cont->first_child, NULL);
}

TEST(ctl_suite, get_bounds, "Should get bounds")
{
    ui_Wrapper *btn = ui_window_new_control(&win, UI_ButtonType),
              *lbl = ui_window_new_control(&win, UI_LabelType),
              *cont = ui_window_new_control(&win, UI_ContainerType);
    ui_window_append(&win, cont);
    ui_control_append(cont, lbl);
    ui_control_append(cont, btn);

    ui_rect_set(&cont->rect, 1, 10, 6, 6);
    ui_rect_set(&btn->rect, 3, 4, 40, 7);
    ui_rect_set(&lbl->rect, 2, 5, 20, 9);
    ui_RenderRect rect = ui_control_get_bounds(cont);

    expectEQ(rect.top_left.x, 1);
    expectEQ(rect.top_left.y, 4);
    expectEQ(rect.bottom_right.x, 40);
    expectEQ(rect.bottom_right.y, 9);
}

TEST(ctl_suite, set_bounds, "Should set bounds")
{
    ui_Wrapper *btn = ui_window_new_control(&win, UI_ButtonType),
              *lbl = ui_window_new_control(&win, UI_LabelType),
              *cont = ui_window_new_control(&win, UI_ContainerType);
    ui_window_append(&win, cont);
    ui_control_append(cont, btn);
    ui_control_append(cont, lbl);

    ui_rect_set(&btn->rect, 3, 4, 40, 7);
    ui_rect_set(&lbl->rect, 2, 5, 20, 9);
    ui_rect_set(&cont->rect, 10, 6, 11, 6);
    ui_RenderRect rect = ui_control_get_bounds(cont);

    expectEQ(rect.top_left.x, 2);
    expectEQ(rect.top_left.y, 4);
    expectEQ(rect.bottom_right.x, 40);
    expectEQ(rect.bottom_right.y, 9);
}

TEST(ctl_suite, set_taborder, "Test setting taborder")
{
    ui_Wrapper *btn = create_ctrl(UI_ButtonType),
              *lbl = create_ctrl(UI_LabelType),
              *cont = create_ctrl(UI_ContainerType),
              *list = create_ctrl(UI_ListType);

    expectTrue(ui_window_set_taborder(&win, list));
    expectEQ((void*)win.first_tab_order->next, win.first_tab_order);
    expectFalse(ui_window_set_taborder(&win, list));
    expectFalse(ui_window_set_taborder(&win, cont));
    expectFalse(ui_window_set_taborder(&win, lbl));
    expectTrue(ui_window_set_taborder(&win, btn));
    expectEQ((void*)win.first_tab_order->control, list);
    expectEQ((void*)win.first_tab_order->next->control, btn);
    expectEQ((void*)win.first_tab_order->next->next, win.first_tab_order);
}

TEST(ctl_suite, nav_fw, "Test without taborder")
{
    ui_Wrapper *btn1 = ui_window_new_control(&win, UI_ButtonType),
              *edit1 = ui_window_new_control(&win, UI_TextEditType),
              *lbl1 = ui_window_new_control(&win, UI_LabelType),
              *btn2 = ui_window_new_control(&win, UI_ButtonType),
              *list2 = ui_window_new_control(&win, UI_ListType),
              *cont1 = ui_window_new_control(&win, UI_ContainerType),
              *cont2 = ui_window_new_control(&win, UI_ContainerType);
    ui_window_append(&win, btn1);
    ui_window_append(&win, cont1);
    ui_control_append(cont1, edit1);
    ui_control_append(cont1, cont2);
    ui_control_append(cont1, lbl1);
    ui_control_append(cont2, list2);
    ui_control_append(cont2, btn2);
    btn2->button->name = "Btn2";
    cont2->container->name = "Cont2";

    // one cycle fw
    ui_window_nav_forward(&win);
    expectEQ((void*)win.focus_control, btn1);
    ui_window_nav_forward(&win);
    expectEQ((void*)win.focus_control, edit1);
    ui_window_nav_forward(&win);
    expectEQ((void*)win.focus_control, list2);
    ui_window_nav_forward(&win);
    expectEQ((void*)win.focus_control, btn2);
    ui_window_nav_forward(&win);
    expectEQ((void*)win.focus_control, btn1);

    // one cycle backward
    ui_window_nav_backward(&win);
    expectEQ((void*)win.focus_control, btn2);
    ui_window_nav_backward(&win);
    expectEQ((void*)win.focus_control, list2);
    ui_window_nav_backward(&win);
    expectEQ((void*)win.focus_control, edit1);
    ui_window_nav_backward(&win);
    expectEQ((void*)win.focus_control, btn1);
    ui_window_nav_backward(&win);
    expectEQ((void*)win.focus_control, btn2);
}


