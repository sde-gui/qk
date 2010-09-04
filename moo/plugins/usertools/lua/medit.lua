local base = _G
local _medit = require("_medit")
module("medit")

-- open file in the editor
function open(filename)
  return _medit.open(filename)
end


local function parse_args(arg)
  if base.type(arg[1]) == 'table' then
    return {cmd_line=arg[1].cmd_line,
            working_dir=arg[1].working_dir,
            env=arg[1].env,
            input=arg[1].input}
  elseif base.type(arg[1]) == 'string' and
         base.type(arg[2]) == 'table'
  then
    return {cmd_line=arg[1],
            working_dir=arg[2].working_dir,
            env=arg[2].env,
            input=arg[2].input}
  else
    return {cmd_line=arg[1],
            working_dir=arg[2],
            env=arg[3],
            input=arg[4]}
  end
end

-- run a command in an output pane
function run_in_pane(...)
  local args = parse_args(arg)
  return _medit.run_in_pane(args.cmd_line, args.working_dir)
end

-- run a command in background
function run_async(...)
  local args = parse_args(arg)
  return _medit.run_async(args.cmd_line, args.working_dir)
end

-- run a command
function run_sync(...)
  local args = parse_args(arg)
  return _medit.run_async(args.cmd_line, args.working_dir, args.input)
end
