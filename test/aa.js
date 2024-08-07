document.addEventListener("DOMContentLoaded", function () {
    const links = document.querySelectorAll("a");
    links.forEach((link) => {
        link.addEventListener("click", (event) => {
            event.preventDefault();
            const target = event.target;
            target.classList.add("active");
            setTimeout(() => {
                target.classList.remove("active");
                target.blur();
            }, 500);
        });
        link.addEventListener("mousemove", (event) => {
            const rect = event.target.getBoundingClientRect();
            const x = event.clientX - rect.left;
            const y = event.clientY - rect.top;
            event.target.style.setProperty('--x', `${x}px`);
            event.target.style.setProperty('--y', `${y}px`);
        });
        link.addEventListener("mouseleave", () => {
            link.style.removeProperty('--x');
            link.style.removeProperty('--y');
        });
    });
});