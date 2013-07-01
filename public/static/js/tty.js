function catch_char(tty, c)
{
    switch (c)
    {
        case 8:
            $(':last-child', tty).remove();
            break;
        case 9:
            $(tty).append('\t');
            break;
        case 13:
            $(tty).append('\n');
            break;
        default:
            $(tty).append(String.fromCharCode(c));
            break;
    }
}

$(document).ready(function(){
    $('.tty').keypress(function(e) {
        switch (e.which) {
            case 8:
            case 9:
                break;
            default:
                e.preventDefault();
                catch_char(e.target, e.which);
                break;
        }
    });
    $('.tty').keydown(function(e) {
        switch (e.which) {
            case 8:
            case 9:
                e.preventDefault();
                catch_char(e.target, e.which);
                break;
        }
    });
});
