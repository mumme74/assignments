#include "testrunner.h"
#include "controls.h"
#include "arena.h"

static mem_Arena arena;
static ui_Window win;
static ui_Wrapper *ctl;

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
}

TEST(ctl_suite, win_create_lbl, "Test create label")
{
    ctl = ui_window_new_control(&win, UI_LabelType);
    expectEQ(ctl->label->name, NULL);
    expectNE((void*)ctl, NULL);
    expectEQ((void*)ctl->window, &win);
    expectEQ((void*)ctl->parent, NULL);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl->first_child, NULL);
    expectEQ(ctl->type, UI_LabelType);
    expectFalse((void*)ctl->dirty);
    expectEQ(ctl->rect.top_left.x, 0);
    expectEQ(ctl->rect.top_left.y, 0);
    expectEQ(ctl->rect.bottom_right.x, 0);
    expectEQ(ctl->rect.bottom_right.y, 0);
    expectNE(ctl->label->bg_color, NULL);
    expectNE(ctl->label->fg_color, NULL);
    expectNE(ctl->label->format, NULL);
    expectEQ((void*)ctl->label->wrapper, ctl);
    expectEQ(ctl->label->shown, true);
    expectEQ((void*)ctl->label->text.arena, &arena);
}


TEST(ctl_suite, win_create_btn, "Test create Button")
{
    ctl = ui_window_new_control(&win, UI_LabelType);
    expectEQ(ctl->button->name, NULL);
    expectNE((void*)ctl, NULL);
    expectEQ((void*)ctl->window, &win);
    expectEQ((void*)ctl->parent, NULL);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl->first_child, NULL);
    expectEQ(ctl->type, UI_LabelType);
    expectFalse((void*)ctl->dirty);
    expectEQ(ctl->rect.top_left.x, 0);
    expectEQ(ctl->rect.top_left.y, 0);
    expectEQ(ctl->rect.bottom_right.x, 0);
    expectEQ(ctl->rect.bottom_right.y, 0);
    expectNE(ctl->button->bg_color, NULL);
    expectNE(ctl->button->fg_color, NULL);
    expectNE(ctl->button->format, NULL);
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
    expectEQ((void*)ctl1->next_sibling, ctl2);
    expectEQ((void*)ctl2->next_sibling, ctl3);
    expectEQ((void*)ctl3->next_sibling, NULL);
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
    expectEQ((void*)win.root, ctl);

    ui_window_remove(&win, ctl);
    expectEQ((void*)win.root, ctl1);

    ui_window_remove(&win, ctl2);
    expectEQ((void*)ctl1->next_sibling, NULL);
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
    expectEQ((void*)ctl->first_child, NULL);

    // 0=textedit, 1=label
    ui_control_insert(cont, ctl1, 0);
    expectEQ((void*)cont->first_child, ctl1);
    expectEQ((void*)ctl->parent, cont);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl1->next_sibling, ctl);
    expectEQ((void*)ctl1->parent, cont);
    expectEQ((void*)ctl1->first_child, NULL);
    expectEQ((void*)ctl->first_child, NULL);

    // 0=textedit, 1=Container, 2=label
    ui_control_insert(cont, ctl2, 1);
    expectEQ((void*)cont->first_child, ctl1);
    expectEQ((void*)ctl1->next_sibling, ctl2);
    expectEQ((void*)ctl2->next_sibling, ctl);
    expectEQ((void*)ctl->next_sibling, NULL);
    expectEQ((void*)ctl2->first_child, NULL);
    expectEQ((void*)ctl1->first_child, NULL);
    expectEQ((void*)ctl->first_child, NULL);

    // 0=textedit, 1=Container, 2=label, 3=List
    ui_control_insert(cont, ctl3, -1);
    expectEQ((void*)cont->first_child, ctl1);
    expectEQ((void*)ctl->next_sibling, ctl3);
    expectEQ((void*)ctl3->next_sibling, NULL);
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
