import graphviz
import json

def parse(u, father, dot, id):
    cur = father + '_' + str(id)
    if u == None:
        return 
    attr = u['attr']
    if 'name' in attr:
        label = attr['name']
        if ('is_array' in attr) and (attr['is_array'] == True):
            label += '[' + str(attr['array_length']) + ']'
        if ('ret_type' in attr):
            label = attr['ret_type'] + ' ' + label;
    elif 'value' in attr:
        label = attr['value']
    elif 'operation' in attr:
        label = attr['operation']
    else:
        label = attr['class'][3:]
        if "Expression" in label:
            label = label[:-10]
    dot.node(cur, str(label))
    dot.edge(father, cur)
    if (u['child'] != None):
        for i, child in enumerate(u['child']):
            parse(child, cur, dot, i)


with open("output.json") as f:
    dict = json.load(f)
    dot = graphviz.Digraph("AST")
    dot.node('root', dict['attr']['class'])
    for i, child in enumerate(dict['child']):
        parse(child, 'root', dot, i)
    dot.render('ast.gv', view=0)