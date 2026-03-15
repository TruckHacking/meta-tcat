from IPython import get_ipython
ipy = get_ipython()
if ipy:
    try:
        ipy.run_line_magic('autosave', '0')
    except Exception:
        pass
